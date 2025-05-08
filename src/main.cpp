/*
    main.cpp
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