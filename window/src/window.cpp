#include "window.hpp"
#include "result.hpp"
#include "sdl_window.hpp"
#include <memory>

Result<std::unique_ptr<WindowSystem>, void> WindowSystemFactory::create(WindowType type) {
    switch (type) {
        case WindowType::SDL3: {
            std::unique_ptr<WindowSystem> ret = std::make_unique<SDLWindowSystem>();
            return Ok{std::move(ret)};
        }
        default:
            return Err{};
    }
}