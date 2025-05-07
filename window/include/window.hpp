#pragma once

#include <memory>
#include <result.hpp>
#include <string_view>
#include <bitflags.hpp>
#include "export_api.h"
#include <optional.hpp>

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

class Window {
private:

public:
#ifdef _WIN32
    virtual HWND getHWND() const = 0;
#endif
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