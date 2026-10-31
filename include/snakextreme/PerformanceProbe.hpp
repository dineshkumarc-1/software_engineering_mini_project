#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace snake {
struct ProbeOptions {
    std::filesystem::path output;
    std::optional<int> seconds;
    bool help{false};
};
// Throws invalid_argument for unknown/incomplete/duplicate options.
ProbeOptions parseProbeOptions(const std::vector<std::string>& arguments);
struct ProbeContext {
    std::string screen;
    std::string mode;
    std::string difficulty;
};
bool operator==(const ProbeContext& a, const ProbeContext& b);
struct FpsWindow {
    std::int64_t startUs;
    std::size_t frames;
    ProbeContext context;
    bool uniform;
    std::size_t session;
};
// One named point on the way to the first frame (or just after it), so
// audio-device and window/graphics start-up can be timed separately (#22).
struct StartupPhase {
    std::string name;
    std::int64_t us;
};
struct InputTiming {
    std::size_t session;
    std::size_t ordinal;
    std::string direction;
    bool accepted;
    std::int64_t dequeuedUs;
    std::int64_t registeredUs;
    std::optional<std::int64_t> appliedUs;
    std::optional<std::int64_t> displayedUs;
};
// SFML-free bounded measurement model. Timestamps are nonnegative steady-clock
// microseconds relative to main entry, NOT simulated engine time or OS event time.
class PerformanceProbe {
public:
    static constexpr std::size_t MaxWindows = 3600;
    static constexpr std::size_t MaxInputs = 4096;
    static constexpr std::int64_t MaxTimestampUs = 86400LL * 1000000;
    static constexpr std::size_t MaxPhases = 16;
    void beginSession();
    void input(std::int64_t dequeued, std::int64_t registered, bool accepted, std::string direction);
    void engineApplied(std::size_t ordinal, std::int64_t now);
    void frame(std::int64_t now, const ProbeContext& context);
    // Records a start-up phase once per name. Out-of-order, out-of-range,
    // unnamed or over-long names count as invalid samples; extra phases past
    // MaxPhases are dropped and counted.
    void phase(const std::string& name, std::int64_t now);
    bool writeReport(const std::filesystem::path& path, std::string& error) const;
    const std::vector<FpsWindow>& windows() const noexcept { return windows_; }
    const std::vector<InputTiming>& inputs() const noexcept { return inputs_; }
    const std::vector<StartupPhase>& phases() const noexcept { return phases_; }
    std::size_t droppedPhases() const noexcept { return droppedPhases_; }
    std::optional<std::int64_t> firstFrameUs() const noexcept { return firstFrameUs_; }
    std::size_t droppedWindows() const noexcept { return droppedWindows_; }
    std::size_t droppedInputs() const noexcept { return droppedInputs_; }
    std::size_t invalidSamples() const noexcept { return invalidSamples_; }
private:
    std::optional<std::int64_t> firstFrameUs_;
    std::int64_t windowStartUs_{0}, lastFrameUs_{0}, lastRegistrationUs_{0};
    ProbeContext currentContext_;
    bool uniform_{true};
    std::size_t frames_{0}, session_{0}, windowSession_{0}, acceptedOrdinal_{0};
    std::size_t droppedWindows_{0}, droppedInputs_{0}, droppedPhases_{0}, invalidSamples_{0};
    std::vector<FpsWindow> windows_;
    std::vector<InputTiming> inputs_;
    std::vector<StartupPhase> phases_;
    std::vector<std::size_t> pending_;
};
} // namespace snake
