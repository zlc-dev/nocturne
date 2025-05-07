#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string_view>
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
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

    SDLWindow(SDL_Window* window) : m_window(window) {
        SDL_GetWindowSize(window, &m_config.w, &m_config.h);
        auto flag = SDL_GetWindowFlags(window);
        if (flag & SDL_WINDOW_RESIZABLE) {
            m_config.flags |= WindowFlags::RESIZEABLE;
        }
        if (flag & SDL_WINDOW_FULLSCREEN) {
            m_config.flags |= WindowFlags::FULLSCREEN;
        }
        
    }
    SDLWindow(WindowConfig config) : m_config(config), m_window(SDLWindow::create(config).unwrap()) {}

#ifdef _WIN32
    HWND getHWND() const override {
        SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
        return (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    }
#endif

    WindowEvent pollEvent() override {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT: {
                    return WindowEvent {
                        .type = WindowEventType::Close,
                        .close_info = WindowEvent::Close{}
                    };
                }
                default: {
                    return WindowEvent {
                        .type = WindowEventType::None,
                        .none_info = WindowEvent::None{}
                    };
                }
            } 
        } else {
            return WindowEvent {
                .type = WindowEventType::None,
                .none_info = WindowEvent::None{}
            };
        }
    }

    const WindowConfig& getConfig() const override {
        return m_config;
    }

    inline static Result<SDL_Window*, void> create(WindowConfig config) {
        
        auto window = SDL_CreateWindow(
            config.title.data(), 
            config.w, 
            config.h, 
            to_sdl_window_flags(config.flags)
        );
        if(!window) return Err{};
        return Ok { std::move(window) };
    }

    ~SDLWindow() {
        SDL_DestroyWindow(m_window);
    }
private:
    WindowConfig m_config;
    SDL_Window* m_window;
};

class SDLWindowSystem: public WindowSystem {
public:
    SDLWindowSystem() = default;

    static Result<std::unique_ptr<WindowSystem>, void> init() {
        if (SDL_Init(SDL_INIT_VIDEO)) {
            std::unique_ptr<WindowSystem> ret = std::make_unique<SDLWindowSystem>();
            return Ok{ std::move(ret) };
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

    ~SDLWindowSystem() override {
        SDL_Quit();
    }
};

