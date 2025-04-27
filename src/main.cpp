#include "result.hpp"
#include "window.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::string_literals;
using namespace std::chrono_literals;

int main(int argc, char* const argv[]) {
    auto window_sys = WindowSystemFactory::crate(WindowType::SDL3).unwrap();
    window_sys->init();
    std::cout << "init\n";
    WindowConfig config {
        .title = "hello",
        .w = 800,
        .h = 600,
        .flags = WindowFlags::RESIZEABLE
    };
    {
        auto window = window_sys->crate(config).unwrap();
        std::this_thread::sleep_for(1s);
    }
    window_sys->deinit();
    return 0;
}