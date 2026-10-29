#include "snakextreme/AppManager.hpp"
#include "snakextreme/PerformanceProbe.hpp"
#include <chrono>
#include <exception>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    const auto mainEntry = std::chrono::steady_clock::now();
    snake::ProbeOptions options;
    try {
        std::vector<std::string> arguments;
        for (int i = 1; i < argc; ++i) arguments.emplace_back(argv[i]);
        options = snake::parseProbeOptions(arguments);
        if (options.help) {
            std::cout << "SnakeXtreme [--perf-probe NEW_REPORT.json [--probe-seconds 1..3600]]\n"
                         "Optional diagnostics; --probe-seconds closes the app after that many seconds\n"
                         "from its first frame. Reports never overwrite existing files.\n";
            return 0;
        }
        if (!options.output.empty() && std::filesystem::exists(options.output)) {
            std::cerr << "Probe output already exists; choose a new report path\n"; return 2;
        }
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 2; }
    snake::PerformanceProbe probe;
    int result = 1;
    try {
        snake::AppManager app(options.output.empty() ? nullptr : &probe, mainEntry, options.seconds);
        result = app.run();
    } catch (const std::exception& error) {
        std::cerr << "SnakeXtreme failed: " << error.what() << '\n';
    } catch (...) { std::cerr << "SnakeXtreme failed with an unexpected error.\n"; }
    if (!options.output.empty()) {
        std::string error;
        if (!probe.writeReport(options.output, error)) { std::cerr << error << '\n'; return 3; }
    }
    return result;
}
