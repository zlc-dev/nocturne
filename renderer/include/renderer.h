/*
    renderer.h
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

#include "export_api.h"
#include "webgpu/webgpu.hpp"
#include <string_view>

#ifdef _WIN32
#include <Windows.h>
#endif

RENDERER_LIB_API int test();

struct WebGPUContext {
    wgpu::Instance instance { nullptr };
    wgpu::Adapter adapter { nullptr };
    wgpu::Device device { nullptr };
    wgpu::Surface surface { nullptr };
    wgpu::RenderPipeline pipeline { nullptr };
};

#ifdef _WIN32

RENDERER_LIB_API wgpu::Surface crateSurfacefromHWND(wgpu::Instance instance, HWND hwnd);

#endif

RENDERER_LIB_API void inspectAdapter(wgpu::Adapter adapter);

RENDERER_LIB_API void inspectDevice(wgpu::Device device);