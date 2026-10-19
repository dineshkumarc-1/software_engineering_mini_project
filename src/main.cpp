#include "snakextreme/AppManager.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        snake::AppManager app;
        return app.run();
    } catch (const std::exception& error) {
        std::cerr << "SnakeXtreme failed: " << error.what() << '\n';
    } catch (...) {
        std::cerr << "SnakeXtreme failed with an unexpected error.\n";
    }
    return 1;
}
