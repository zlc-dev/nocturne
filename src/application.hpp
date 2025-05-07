#pragma once

#include "renderer.h"
#include "webgpu/webgpu.hpp"
#include "window.hpp"
#include <memory>

extern "C" const char _binary_assets_wgsl_test_wgsl_start[];

class Application {
public:
    Application() = default;

    inline void initialize(std::unique_ptr<Window>&& window) {
        m_window = std::move(window);
        wgpu::InstanceDescriptor inst_desc = {};
        inst_desc.nextInChain = nullptr;
        m_instance = wgpu::createInstance(inst_desc);
#ifdef _WIN32
        m_surface = crateSurfacefromHWND(m_instance, m_window->getHWND());
#endif

        wgpu::RequestAdapterOptions adapter_opts = {};
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
        render_pass_color_attachment.clearValue = wgpu::Color{ 0xf3 / 255.0, 0x59 / 255.0, 0x7c / 255.0, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
        render_pass_color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &render_pass_color_attachment;
        render_pass_desc.depthStencilAttachment = nullptr;
        render_pass_desc.timestampWrites = nullptr;

        wgpu::RenderPassEncoder render_pass_encoder = cmd_encoder.beginRenderPass(render_pass_desc);

        render_pass_encoder.setPipeline(m_render_pipeline);
        render_pass_encoder.draw(3, 1, 0, 0);
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
        m_instance.release();
        m_device.release();
        m_surface.release();
        m_queue.release();
        m_render_pipeline.release();
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

        render_pipline_desc.vertex.bufferCount = 0;
        render_pipline_desc.vertex.buffers = nullptr;

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

private:
    std::unique_ptr<Window> m_window { nullptr };
    std::unique_ptr<wgpu::ErrorCallback> m_device_err_callback_holder { nullptr };
    wgpu::Instance m_instance { nullptr };
    wgpu::Device m_device { nullptr };
    wgpu::Surface m_surface { nullptr };
    wgpu::Queue m_queue { nullptr };
    wgpu::RenderPipeline m_render_pipeline { nullptr };
    wgpu::TextureFormat m_surface_format { wgpu::TextureFormat::Undefined };
    bool m_need_close { false };
};
