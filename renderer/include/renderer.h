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