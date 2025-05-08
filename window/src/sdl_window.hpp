/*
    sdl_window.hpp
    Copyright (C) 2025 zlc-dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
*/

#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string_view>
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_properties.h"
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

#if defined (SDL_PLATFORM_WIN32)
    WindowDisplay getDisplay() const override {
        SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
        void *hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        return WindowDisplay {
            .type = WindowDisplay::Type::HWND,
            .hwnd = WindowDisplay::HWND {
                .hwnd = hwnd,
                .hinstance = GetModuleHandle(NULL)
            }
        };
    }
#elif defined (SDL_PLATFORM_LINUX)
    WindowDisplay getDisplay() const override {
        SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
            void *x11_display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
            uint64_t x11_window = (uint64_t)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
            return WindowDisplay {
                .type = WindowDisplay::Type::X11,
                .x11 = WindowDisplay::X11 {
                    .display = x11_display,
                    .window = x11_window
                }
            };
        } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
            void *wayland_display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
            void *wayland_surface = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
            return WindowDisplay {
                .type = WindowDisplay::Type::Wayland,
                .wayland = WindowDisplay::Wayland {
                    .display = wayland_display,
                    .surface = wayland_surface
                }
            };
        } else {
            abort();
        }
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

