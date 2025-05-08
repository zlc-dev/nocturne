/*
    application.hpp
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

#include "model_loader.hpp"
#include "renderer.h"
#include "webgpu/webgpu.hpp"
#include "window.hpp"
#include <array>
#include <cstdlib>
#include <memory>
#include <stdlib.h>

extern "C" const char _binary_assets_wgsl_test_wgsl_start[];

extern "C" const char _binary_assets_model_monkey_head_obj_start[];
extern "C" const char _binary_assets_model_monkey_head_obj_end[];

static const std::array<float, 12> vertex_data = {
    -0.5, -0.5,
    +0.5, -0.5,
    +0.0, +0.5,
    -0.55f, -0.5,
    -0.05f, +0.5,
    -0.55f, +0.5
};

static const std::array<float, 12> position_data {
    -0.5, -0.5,
    +0.5, -0.5,
    +0.0, +0.5,
    -0.55f, -0.5,
    -0.05f, +0.5,
    -0.55f, +0.5
};

static const std::array<float, 18> color_data {
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 0.0, 1.0,
    0.0, 1.0, 1.0
};

inline static wgpu::Surface crateSurfacefromWindow(wgpu::Instance instance, const Window& window) {
    auto display = window.getDisplay();
    switch (display.type) {
        case WindowDisplay::Type::HWND: {
            wgpu::SurfaceDescriptorFromWindowsHWND from_windows_display;
            from_windows_display.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
            from_windows_display.chain.next = NULL;
            from_windows_display.hinstance = display.hwnd.hinstance;
            from_windows_display.hwnd = display.hwnd.hwnd;
            
            wgpu::SurfaceDescriptor surface_descriptor;
            surface_descriptor.nextInChain = &from_windows_display.chain;
            surface_descriptor.label = nullptr;
            return instance.createSurface(surface_descriptor);
        }
        case WindowDisplay::Type::X11: {
            wgpu::SurfaceDescriptorFromXlibWindow from_windows_display;
            from_windows_display.chain.sType = wgpu::SType::SurfaceDescriptorFromXlibWindow;
            from_windows_display.chain.next = NULL;
            from_windows_display.display = display.x11.display;
            from_windows_display.window = display.x11.window;

            wgpu::SurfaceDescriptor surface_descriptor;
            surface_descriptor.nextInChain = &from_windows_display.chain;
            surface_descriptor.label = nullptr;
            return instance.createSurface(surface_descriptor);
        }
        case WindowDisplay::Type::Wayland: {
            wgpu::SurfaceDescriptorFromWaylandSurface from_windows_display;
            from_windows_display.chain.sType = wgpu::SType::SurfaceDescriptorFromWaylandSurface;
            from_windows_display.chain.next = NULL;
            from_windows_display.display = display.wayland.display;
            from_windows_display.surface = display.wayland.surface;

            wgpu::SurfaceDescriptor surface_descriptor;
            surface_descriptor.nextInChain = &from_windows_display.chain;
            surface_descriptor.label = nullptr;
            return instance.createSurface(surface_descriptor);
        }
    }
}

class Application {
public:
    Application() = default;

    inline void initialize(std::unique_ptr<Window>&& window) {
        m_window = std::move(window);
        wgpu::InstanceDescriptor inst_desc = {};
        inst_desc.nextInChain = nullptr;
        m_instance = wgpu::createInstance(inst_desc);
        m_surface = crateSurfacefromWindow(m_instance, *m_window);
        wgpu::RequestAdapterOptions adapter_opts = {};
        adapter_opts.powerPreference = wgpu::PowerPreference::HighPerformance;
        wgpu::Adapter adapter = m_instance.requestAdapter(adapter_opts);
        inspectAdapter(adapter);
        wgpu::DeviceDescriptor dev_desc = {};
        dev_desc.nextInChain = nullptr;
        dev_desc.label = "My Device";
        dev_desc.requiredFeatureCount = 0; // we do not require any specific feature
        dev_desc.requiredLimits = nullptr; // we do not require any specific limit
        dev_desc.defaultQueue.nextInChain = nullptr;
        dev_desc.defaultQueue.label = "The default queue";

#ifdef WEBGPU_BACKEND_DAWN
        wgpu::DeviceLostCallbackInfo dev_lost_callback_info = {};
        dev_lost_callback_info.nextInChain = nullptr;
        dev_lost_callback_info.callback = [](const WGPUDevice *device, WGPUDeviceLostReason reason, const char *message, void * /* p_user_data */) {
            std::cout << "Device lost: reason " << reason;
            if (message) std::cout << " (" << message << ")";
            std::cout << '\n';
        };
        dev_lost_callback_info.mode = wgpu::CallbackMode::AllowProcessEvents;
        dev_lost_callback_info.userdata = nullptr;
        dev_desc.deviceLostCallbackInfo = dev_lost_callback_info;

        wgpu::DawnTogglesDescriptor toggles;
        toggles.chain.next = nullptr;
        toggles.chain.sType = WGPUSType_DawnTogglesDescriptor;
        toggles.disabledToggleCount = 0;
        toggles.enabledToggleCount = 1;
        const char* toggle_name = "enable_immediate_error_handling";
        toggles.enabledToggles = &toggle_name;
        dev_desc.nextInChain = &toggles.chain;
#else
        dev_desc.deviceLostCallback = [](WGPUDeviceLostReason reason, const char *message, void * /* p_user_data */) {
            std::cout << "Device lost: reason " << reason;
            if (message) std::cout << " (" << message << ")";
            std::cout << '\n';
        };
#endif // WEBGPU_BACKEND_DAWN

        m_device = adapter.requestDevice(dev_desc);

        auto on_dev_error = [](wgpu::ErrorType type, char const* message) {
            std::cout << "Uncaptured device error: type " << type;
            if (message) std::cout << " (" << message << ")";
            std::cout << '\n';
            abort();
        };
        m_device_err_callback_holder = m_device.setUncapturedErrorCallback(std::move(on_dev_error));

        inspectDevice(m_device);

        wgpu::SurfaceConfiguration surface_config = {};
        auto config = m_window->getConfig();
        surface_config.width = config.w;
        surface_config.height = config.h;
        surface_config.format = m_surface_format = m_surface.getPreferredFormat(adapter);
        surface_config.usage = wgpu::TextureUsage::RenderAttachment;
        surface_config.viewFormatCount = 0;
        surface_config.viewFormats = nullptr;
        surface_config.device = m_device;
        surface_config.presentMode = wgpu::PresentMode::Fifo;
        surface_config.alphaMode = wgpu::CompositeAlphaMode::Auto;
        m_surface.configure(surface_config);

        adapter.release();

        m_queue = m_device.getQueue();

        initializeRenderPipline();

        initializeBuffer();
    }

    inline void mainLoop() {
        auto event = m_window->pollEvent();
        switch (event.type) {
            case WindowEventType::Close: {
                m_need_close = true;
                return;
            }
            default: break;
        }

        // get the surface texture
        wgpu::SurfaceTexture surface_texture;
        m_surface.getCurrentTexture(&surface_texture);
        wgpu::Texture texture = surface_texture.texture;
        // Create a view for this surface texture
        wgpu::TextureViewDescriptor texture_view_desc = {};
        texture_view_desc.nextInChain = nullptr;
        texture_view_desc.label = "Surface texture view";
        texture_view_desc.format = texture.getFormat();
        texture_view_desc.dimension = wgpu::TextureViewDimension::_2D;
        texture_view_desc.baseMipLevel = 0;
        texture_view_desc.mipLevelCount = 1;
        texture_view_desc.baseArrayLayer = 0;
        texture_view_desc.arrayLayerCount = 1;
        texture_view_desc.aspect = wgpu::TextureAspect::All;
        wgpu::TextureView target_view = texture.createView(texture_view_desc);

        if(!target_view) {
            m_need_close = true;
            return;
        }

        wgpu::CommandEncoderDescriptor cmd_encoder_desc = {};
        cmd_encoder_desc.nextInChain = nullptr;
        cmd_encoder_desc.label = "My command encoder";
        wgpu::CommandEncoder cmd_encoder = m_device.createCommandEncoder(cmd_encoder_desc);

        wgpu::RenderPassDescriptor render_pass_desc = {};
        render_pass_desc.nextInChain = nullptr;
        render_pass_desc.label = "My render pass";

        wgpu::RenderPassColorAttachment render_pass_color_attachment = {};
        
        render_pass_color_attachment.nextInChain = nullptr;
        render_pass_color_attachment.view = target_view;
        render_pass_color_attachment.resolveTarget = nullptr;
        render_pass_color_attachment.loadOp = wgpu::LoadOp::Clear;
        render_pass_color_attachment.storeOp = wgpu::StoreOp::Store;
        // render_pass_color_attachment.clearValue = wgpu::Color{ 0xf3 / 255.0, 0x59 / 255.0, 0x7c / 255.0, 1.0 };
        render_pass_color_attachment.clearValue = wgpu::Color{ 0.15, 0.15, 0.15, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
        render_pass_color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &render_pass_color_attachment;
        render_pass_desc.depthStencilAttachment = nullptr;
        render_pass_desc.timestampWrites = nullptr;

        wgpu::RenderPassEncoder render_pass_encoder = cmd_encoder.beginRenderPass(render_pass_desc);

        render_pass_encoder.setPipeline(m_render_pipeline);
        render_pass_encoder.setVertexBuffer(0, m_model_vertices_buffer, 0, m_model_vertices_buffer.getSize());
        render_pass_encoder.setIndexBuffer(m_model_indices_buffer, wgpu::IndexFormat::Uint32, 0, m_model_indices_buffer.getSize());
        render_pass_encoder.drawIndexed(m_index_count, 1, 0, 0, 0);
        render_pass_encoder.end();
        render_pass_encoder.release();

        wgpu::CommandBufferDescriptor cmd_buf_desc = {};
        cmd_buf_desc.nextInChain = nullptr;
        cmd_buf_desc.label = "Command buffer";
        wgpu::CommandBuffer cmd_buf = cmd_encoder.finish(cmd_buf_desc);
        cmd_encoder.release();
        m_queue.submit(cmd_buf);
        cmd_buf.release();

        target_view.release();
#ifndef __EMSCRIPTEN__
        m_surface.present();
#endif
#ifndef WEBGPU_BACKEND_WGPU
        // We no longer need the texture, only its view
        // (NB: with wgpu-native, surface textures must not be manually released)
        texture.release();
#endif // WEBGPU_BACKEND_WGPU

#if defined(WEBGPU_BACKEND_DAWN)
        m_device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
        m_device.poll(false);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
        emscripten_sleep(100);
#endif

    }

    inline bool needClose() const {
        return m_need_close;
    }

    inline ~Application() {
        m_model_vertices_buffer.release();
        m_model_indices_buffer.release();
        m_render_pipeline.release();
        m_queue.release();
        m_surface.release();
        m_device.release();
        m_instance.release();
    }

private:

    inline void initializeRenderPipline() {
        wgpu::ShaderModuleDescriptor shader_module_desc = {};
#ifdef WEBGPU_BACKEND_WGPU
        shader_module_desc.hintCount = 0;
        shader_module_desc.hints = nullptr;
#endif
        wgpu::ShaderModuleWGSLDescriptor shader_code_desc = {};

        shader_code_desc.chain.next = nullptr;
        shader_code_desc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;

        shader_module_desc.nextInChain = &shader_code_desc.chain;
        shader_code_desc.code = _binary_assets_wgsl_test_wgsl_start;
        
        wgpu::ShaderModule shader_module = m_device.createShaderModule(shader_module_desc);

        wgpu::RenderPipelineDescriptor render_pipline_desc = {};

        wgpu::VertexBufferLayout vertex_buffer_layout[1] = {{}};
        wgpu::VertexAttribute vertex_attr[1] = {{}};
        vertex_attr[0].format = wgpu::VertexFormat::Float32x3;
        vertex_attr[0].offset = 0;
        vertex_attr[0].shaderLocation = 0;

        vertex_buffer_layout[0].attributeCount = 1;
        vertex_buffer_layout[0].attributes = vertex_attr;
        vertex_buffer_layout[0].arrayStride = 3*sizeof(float);
        vertex_buffer_layout[0].stepMode = wgpu::VertexStepMode::Vertex;

        render_pipline_desc.vertex.bufferCount = 1;
        render_pipline_desc.vertex.buffers = vertex_buffer_layout;

        render_pipline_desc.vertex.module = shader_module;
        render_pipline_desc.vertex.entryPoint = "vs_main";
        render_pipline_desc.vertex.constantCount = 0;
        render_pipline_desc.vertex.constants = nullptr;

        render_pipline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        render_pipline_desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
        render_pipline_desc.primitive.frontFace = wgpu::FrontFace::CCW;
        render_pipline_desc.primitive.cullMode = wgpu::CullMode::None;

        wgpu::FragmentState frag_state = {};
        frag_state.module = shader_module;
        frag_state.entryPoint = "fs_main";
        frag_state.constantCount = 0;
        frag_state.constants = nullptr;

        wgpu::BlendState blend_state = {};
        blend_state.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        blend_state.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
        blend_state.color.operation = wgpu::BlendOperation::Add;
        blend_state.alpha.srcFactor = wgpu::BlendFactor::Zero;
        blend_state.alpha.dstFactor = wgpu::BlendFactor::One;
        blend_state.alpha.operation = wgpu::BlendOperation::Add;

        wgpu::ColorTargetState color_target_state = {};
        color_target_state.format = m_surface_format;
        color_target_state.blend = &blend_state;
        color_target_state.writeMask = wgpu::ColorWriteMask::All;

        frag_state.targetCount = 1;
        frag_state.targets = &color_target_state;
        
        render_pipline_desc.fragment = &frag_state;
        render_pipline_desc.depthStencil = nullptr;

        render_pipline_desc.multisample.count = 1;
        render_pipline_desc.multisample.mask = ~0u;

        render_pipline_desc.multisample.alphaToCoverageEnabled = false;
        render_pipline_desc.layout = nullptr;

        m_render_pipeline = m_device.createRenderPipeline(render_pipline_desc);

        shader_module.release();
    }

    inline void initializeBuffer() {
        Model model;
        model.loadModelFromMemory(
            (const void *)_binary_assets_model_monkey_head_obj_start, 
            _binary_assets_model_monkey_head_obj_end - _binary_assets_model_monkey_head_obj_start
        );

        wgpu::BufferDescriptor buffer_desc = {};
        buffer_desc.size = sizeof(vertex_data);
        buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
        buffer_desc.mappedAtCreation = false;

        buffer_desc.size = model.m_vertices.size() * sizeof(model.m_vertices[0]);
        buffer_desc.label = "Model vertices";
        m_model_vertices_buffer = m_device.createBuffer(buffer_desc);
        m_queue.writeBuffer(m_model_vertices_buffer, 0, model.m_vertices.data(), buffer_desc.size);

        buffer_desc.size = model.m_indices.size() * sizeof(model.m_indices[0]);
        buffer_desc.label = "Model indices";
        buffer_desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
        m_model_indices_buffer = m_device.createBuffer(buffer_desc);
        m_queue.writeBuffer(m_model_indices_buffer, 0, model.m_indices.data(), buffer_desc.size);

        m_index_count = model.m_indices.size();
    }

private:
    std::unique_ptr<Window> m_window { nullptr };
    std::unique_ptr<wgpu::ErrorCallback> m_device_err_callback_holder { nullptr };
    wgpu::Instance m_instance { nullptr };
    wgpu::Device m_device { nullptr };
    wgpu::Surface m_surface { nullptr };
    wgpu::Queue m_queue { nullptr };
    wgpu::RenderPipeline m_render_pipeline { nullptr };
    wgpu::TextureFormat m_surface_format { wgpu::TextureFormat::Undefined };
    wgpu::Buffer m_model_vertices_buffer { nullptr };
    wgpu::Buffer m_model_indices_buffer { nullptr };
    unsigned m_index_count = 0;
    bool m_need_close { false };
};
