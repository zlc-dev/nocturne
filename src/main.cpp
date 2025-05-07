#include "application.hpp"
#include "result.hpp"
#include "window.hpp"

using namespace std::string_literals;

int main(int argc, char* const argv[]) {

    auto window_sys = WindowSystemFactory::create(WindowType::SDL3)
        .expect("cannot create window system");
    WindowConfig config {
        .title = "hello",
        .w = 800,
        .h = 600,
        .flags = WindowFlags::RESIZEABLE
    };

    {
        auto window = window_sys->create(config).expect("cannot create window");
        Application app;
        app.initialize(std::move(window));
        while(!app.needClose()) {
            app.mainLoop();
        }
    }

    return 0;
}