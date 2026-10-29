#include "snakextreme/PerformanceProbe.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <charconv>
#include <cstdio>
#include <memory>
#include <stdexcept>

namespace snake {
namespace {
bool validTime(std::int64_t value) { return value >= 0 && value <= PerformanceProbe::MaxTimestampUs; }
struct FileCloser { void operator()(std::FILE* file) const noexcept { if (file) (void)std::fclose(file); } };
}
ProbeOptions parseProbeOptions(const std::vector<std::string>& args) {
    ProbeOptions options;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--help" && args.size() == 1) { options.help = true; return options; }
        if (args[i] == "--perf-probe" && options.output.empty() && i + 1 < args.size()) {
            options.output = args[++i];
            if (options.output.empty() || args[i].rfind("--", 0) == 0) throw std::invalid_argument("Invalid probe output path");
        } else if (args[i] == "--probe-seconds" && !options.seconds && i + 1 < args.size()) {
            const auto& value = args[++i]; int seconds = 0;
            const auto parsed = std::from_chars(value.data(), value.data() + value.size(), seconds);
            if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || seconds < 1 || seconds > 3600)
                throw std::invalid_argument("Probe seconds must be an integer 1..3600");
            options.seconds = seconds;
        } else throw std::invalid_argument("Unknown, duplicate or incomplete option: " + args[i]);
    }
    if (options.seconds && options.output.empty()) throw std::invalid_argument("--probe-seconds requires --perf-probe");
    return options;
}
bool operator==(const ProbeContext& a, const ProbeContext& b) {
    return a.screen == b.screen && a.mode == b.mode && a.difficulty == b.difficulty;
}
void PerformanceProbe::beginSession() { ++session_; acceptedOrdinal_ = 0; pending_.clear(); }
void PerformanceProbe::input(std::int64_t dequeued, std::int64_t registered, bool accepted, std::string direction) {
    if (accepted) ++acceptedOrdinal_;
    if (!validTime(dequeued) || !validTime(registered) || registered < dequeued || dequeued < lastRegistrationUs_) {
        ++invalidSamples_; return;
    }
    lastRegistrationUs_ = registered;
    if (inputs_.size() == MaxInputs) { ++droppedInputs_; return; }
    if (direction != "Up" && direction != "Down" && direction != "Left" && direction != "Right") {
        ++invalidSamples_; return;
    }
    inputs_.push_back({session_, accepted ? acceptedOrdinal_ : 0, std::move(direction), accepted,
                       dequeued, registered, std::nullopt, std::nullopt});
    if (accepted) pending_.push_back(inputs_.size() - 1);
}
void PerformanceProbe::engineApplied(std::size_t ordinal, std::int64_t now) {
    if (!validTime(now)) { ++invalidSamples_; return; }
    for (auto index : pending_) {
        auto& sample = inputs_[index];
        if (!sample.appliedUs && sample.ordinal <= ordinal) {
            if (now < sample.registeredUs) { ++invalidSamples_; continue; }
            sample.appliedUs = now;
        }
    }
}
void PerformanceProbe::frame(std::int64_t now, const ProbeContext& context) {
    if (!validTime(now) || (firstFrameUs_ && now < lastFrameUs_)) { ++invalidSamples_; return; }
    if (!firstFrameUs_) { firstFrameUs_ = now; windowStartUs_ = now; currentContext_ = context; windowSession_ = session_; }
    const bool contextChanged = !(currentContext_ == context) || windowSession_ != session_;
    if (contextChanged) uniform_ = false;
    while (now - windowStartUs_ >= 1000000) {
        if (windows_.size() < MaxWindows) windows_.push_back({windowStartUs_, frames_, currentContext_, uniform_, windowSession_});
        else ++droppedWindows_;
        windowStartUs_ += 1000000; frames_ = 0;
        // Context through a stall is unknown: never assign it to a gameplay
        // acceptance window merely from the frame after the stall.
        uniform_ = !contextChanged;
        if (windows_.size() == MaxWindows && now - windowStartUs_ >= 1000000) {
            const auto skipped = (now - windowStartUs_) / 1000000;
            droppedWindows_ += static_cast<std::size_t>(skipped); windowStartUs_ += skipped * 1000000;
        }
    }
    currentContext_ = context; windowSession_ = session_;
    ++frames_; lastFrameUs_ = now;
    for (auto index : pending_) {
        auto& sample = inputs_[index];
        if (sample.appliedUs && *sample.appliedUs <= now) sample.displayedUs = now;
    }
    pending_.erase(std::remove_if(pending_.begin(), pending_.end(), [this](auto i) { return inputs_[i].displayedUs.has_value(); }), pending_.end());
}
bool PerformanceProbe::writeReport(const std::filesystem::path& path, std::string& error) const {
    error.clear();
    try {
        using Json = nlohmann::json;
        Json report{{"schema_version", 1}, {"clock", "steady_clock_us_since_main"},
            {"input_scope", "SFML_dequeue_to_engine_queue_registration_lower_bound"},
            {"display_scope", "first_display_return_after_engine_consumed_input"},
            {"acceptance", "not_assessed"}, {"powerups_implemented", false},
            {"first_frame_us", firstFrameUs_ ? Json(*firstFrameUs_) : Json(nullptr)},
            {"dropped_windows", droppedWindows_}, {"dropped_inputs", droppedInputs_},
            {"invalid_samples", invalidSamples_}, {"fps_windows", Json::array()}, {"inputs", Json::array()}};
        for (const auto& w : windows_) report["fps_windows"].push_back({{"start_us", w.startUs},
            {"duration_us", 1000000}, {"frames", w.frames}, {"screen", w.context.screen},
            {"mode", w.context.mode}, {"difficulty", w.context.difficulty}, {"uniform_context", w.uniform}, {"session", w.session}});
        report["partial_window"] = {{"start_us", windowStartUs_}, {"duration_us", firstFrameUs_ ? lastFrameUs_ - windowStartUs_ : 0},
            {"frames", frames_}, {"screen", currentContext_.screen}, {"uniform_context", uniform_}};
        for (const auto& i : inputs_) report["inputs"].push_back({{"session", i.session}, {"ordinal", i.ordinal},
            {"direction", i.direction}, {"accepted", i.accepted}, {"dequeued_us", i.dequeuedUs},
            {"registered_us", i.registeredUs}, {"applied_us", i.appliedUs ? Json(*i.appliedUs) : Json(nullptr)},
            {"displayed_us", i.displayedUs ? Json(*i.displayedUs) : Json(nullptr)}});
        const auto bytes = report.dump(2) + "\n";
        std::unique_ptr<std::FILE, FileCloser> file;
#if defined(_WIN32)
        std::FILE* handle = nullptr;
        if (_wfopen_s(&handle, path.c_str(), L"wbx") == 0) file.reset(handle);
#else
        file.reset(std::fopen(path.c_str(), "wbx"));
#endif
        if (!file) { error = "Probe report must be a new writable file"; return false; }
        const bool written = std::fwrite(bytes.data(), 1, bytes.size(), file.get()) == bytes.size();
        const bool flushed = std::fflush(file.get()) == 0;
        const bool closed = std::fclose(file.release()) == 0;
        if (!written || !flushed || !closed) { error = "Probe report incomplete; discard it"; return false; }
        return true;
    } catch (const std::exception&) { error = "Probe report failed"; return false; }
}
} // namespace snake
