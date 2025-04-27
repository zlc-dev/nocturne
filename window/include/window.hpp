#pragma once

#include <memory>
#include <result.hpp>
#include <string_view>
#include <bitflags.hpp>
#include "export_api.h"

BEGIN_BIT_TAGS(WindowFlags, unsigned int)
    DEF_BIT_TAG(RESIZEABLE, 0)
    DEF_BIT_TAG(FULLSCREEN, 1)
END_BIT_TAGS

enum class WindowType {
    SDL3,
    GLFW,
};

struct WindowConfig {
    std::string_view title;
    int w, h;
    WindowFlags flags;
};

class Window {
public:
    virtual ~Window() = default;
};

class WindowSystem {
public:
    virtual Result<void, void> init() = 0;

    virtual Result<std::unique_ptr<Window>, void> create(WindowConfig config) = 0;

    virtual void dispose() = 0;

    virtual ~WindowSystem() = default;
};

class WINDOW_LIB_API WindowSystemFactory {
public:
    static Result<std::unique_ptr<WindowSystem>, void> create(WindowType type);
};