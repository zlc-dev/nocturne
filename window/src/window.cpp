/*
    window.cpp
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