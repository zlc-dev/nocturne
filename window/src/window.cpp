#include "window.hpp"
#include "result.hpp"
#include "sdl_window.hpp"
#include <memory>

Result<std::unique_ptr<WindowSystem>, void> WindowSystemFactory::create(WindowType type) {
    switch (type) {
        case WindowType::SDL3: {
            return SDLWindowSystem::init();
        }
        default:
            return Err{};
    }
}