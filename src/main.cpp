#include "result.hpp"
#include "window.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <type_traits>

using namespace std::string_literals;
using namespace std::chrono_literals;

int main(int argc, char* const argv[]) {
    auto window_sys = WindowSystemFactory::crate(WindowType::SDL3)
        .expect("cannot crate window system");
    (void)window_sys->init();
    std::cout << "init\n";
    WindowConfig config {
        .title = "hello",
        .w = 800,
        .h = 600,
        .flags = WindowFlags::RESIZEABLE
    };
    {
        auto window = window_sys->crate(config).expect("cannot crate window");
        std::this_thread::sleep_for(1s);
    }

    Result<int, bool> res = Ok{10};
    (void)std::move(res).map([](int x) { std::cout << x << '\n'; })
                        .map([](void) {std::cout << "void\n"; });

    window_sys->deinit();
    return 0;
}