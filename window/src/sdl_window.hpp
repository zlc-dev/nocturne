#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string_view>
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_video.h"
#include "result.hpp"
#include "window.hpp"

inline static SDL_WindowFlags to_sdl_window_flags(WindowFlags flags) {
    SDL_WindowFlags ret = 0;
    if(flags.constains(WindowFlags::RESIZEABLE)) {
        ret |= SDL_WINDOW_RESIZABLE;
    }
    if(flags.constains(WindowFlags::FULLSCREEN)) {
        ret |= SDL_WINDOW_FULLSCREEN;
    }
    return ret;
}

class SDLWindow: public Window {
public:

    SDLWindow(SDL_Window* window) : m_window(window) {}
    SDLWindow(WindowConfig config) : m_window(SDLWindow::create(config).unwrap()) {}

    inline static Result<SDL_Window*, void> create(WindowConfig config) {
        auto window = SDL_CreateWindow(
            config.title.data(), 
            config.w, 
            config.h, 
            to_sdl_window_flags(config.flags)
        );
        if(!window) return Err{};
        return Ok {window};
    }

    ~SDLWindow() {
        SDL_DestroyWindow(m_window);
    }
private:
    SDL_Window* m_window;
};

class SDLWindowSystem: public WindowSystem {
public:
    SDLWindowSystem() = default;

    Result<void, void> init() override {
        SDL_Log("SDL init\n");
        if (SDL_Init(SDL_INIT_VIDEO)) {
            return Ok{};
        } else {
            return Err{};
        }
    }

    Result<std::unique_ptr<Window>, void> create(WindowConfig config) override {
        return SDLWindow::create(config).map([](SDL_Window* window) {
            std::unique_ptr<Window> ret = std::make_unique<SDLWindow>(window);
            return ret; 
        });
    }

    void dispose() override {
        SDL_Log("SDL dispose\n");
        SDL_Quit();
    }
};
