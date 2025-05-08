/*
    window.hpp
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

#include <cstdint>
#include <memory>
#include <result.hpp>
#include <string_view>
#include <bitflags.hpp>
#include "export_api.h"
#include <optional.hpp>
#include <SDL3/SDL_platform_defines.h>

#ifdef _WIN32
#include <Windows.h>
#endif

BEGIN_BIT_TAGS(WindowFlags, unsigned int, 0)
    DEF_TAG(RESIZEABLE, 1 << 0);
    DEF_TAG(FULLSCREEN, 1 << 1);
END_BIT_TAGS;

enum class WindowType {
    SDL3,
    GLFW,
};

struct WindowConfig {
    std::string_view title;
    int w, h;
    WindowFlags flags;
};

enum class WindowEventType {
    None, Close,
};

struct WindowEvent {
    struct None {};
    struct Close {};
    
    struct Resize {
        int w, h;
    };

    WindowEventType type;

    union {
        None none_info;
        Close close_info;
        Resize resize_info;
    };
};

struct WindowDisplay {
    enum class Type {
        HWND,
        X11,
        Wayland
    };

    struct HWND {
        void* hwnd;
        void* hinstance;
    };

    struct X11 {
        void* display;
        uint64_t window;
    };

    struct Wayland {
        void* display;
        void* surface;
    };

    Type type;
    union {
        HWND hwnd;
        X11 x11;
        Wayland wayland;
    };
};

class Window {
private:

public:
    virtual WindowDisplay getDisplay() const = 0;

    virtual WindowEvent pollEvent() = 0;

    virtual const WindowConfig& getConfig() const = 0;

    virtual ~Window() = default;
};


class WindowSystem {
public:
    virtual Result<std::unique_ptr<Window>, void> create(WindowConfig config) = 0;

    virtual ~WindowSystem() = default;
};

class WINDOW_LIB_API WindowSystemFactory {
public:
    static Result<std::unique_ptr<WindowSystem>, void> create(WindowType type);
};