#include "renderer.h"
#include "result.hpp"
#include "webgpu/webgpu.h"
#include "webgpu/webgpu.hpp"
#include "window.hpp"
#include <windef.h>

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

        wgpu::InstanceDescriptor inst_desc = {};
        inst_desc.nextInChain = nullptr;
        wgpu::Instance instance = wgpu::createInstance(inst_desc);
        wgpu::Surface surface = crateSurfacefromHWND(instance, window->getHWND());
        wgpu::RequestAdapterOptions adapter_opts = {};
        wgpu::Adapter adapter = instance.requestAdapter(adapter_opts);
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
        wgpu::Device device = adapter.requestDevice(dev_desc);
        
        auto on_dev_error = [](wgpu::ErrorType type, char const* message) {
            std::cout << "Uncaptured device error: type " << type;
            if (message) std::cout << " (" << message << ")";
            std::cout << '\n';
        };
        auto callback_holder1 = device.setUncapturedErrorCallback(std::move(on_dev_error));
        inspectDevice(device);

        wgpu::Queue queue = device.getQueue();

        wgpu::SurfaceConfiguration surface_config = {};
        surface_config.width = config.w;
        surface_config.height = config.h;
        surface_config.format = surface.getPreferredFormat(adapter);
        surface_config.usage = wgpu::TextureUsage::RenderAttachment;
        surface_config.viewFormatCount = 0;
        surface_config.viewFormats = nullptr;
        surface_config.device = device;
        surface_config.presentMode = wgpu::PresentMode::Fifo;
        surface_config.alphaMode = wgpu::CompositeAlphaMode::Auto;
        surface.configure(surface_config);

        adapter.release();

        for(;;) {
            auto event = window->pollEvent();
            switch (event.type) {
                case WindowEventType::Close: {
                    goto END;
                }
                default: break;
            }

            // get the surface texture
            wgpu::SurfaceTexture surface_texture;
            surface.getCurrentTexture(&surface_texture);
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
                goto END;
            }

            wgpu::CommandEncoderDescriptor cmd_encoder_desc = {};
            cmd_encoder_desc.nextInChain = nullptr;
            cmd_encoder_desc.label = "My command encoder";
            wgpu::CommandEncoder cmd_encoder = device.createCommandEncoder(cmd_encoder_desc);

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
            render_pass_encoder.end();
            render_pass_encoder.release();

            wgpu::CommandBufferDescriptor cmd_buf_desc = {};
            cmd_buf_desc.nextInChain = nullptr;
            cmd_buf_desc.label = "Command buffer";
            wgpu::CommandBuffer cmd_buf = cmd_encoder.finish(cmd_buf_desc);
            cmd_encoder.release();
            queue.submit(cmd_buf);
            cmd_buf.release();

            target_view.release();
#ifndef __EMSCRIPTEN__
            surface.present();
#endif
#ifndef WEBGPU_BACKEND_WGPU
            // We no longer need the texture, only its view
            // (NB: with wgpu-native, surface textures must not be manually released)
            texture.release();
#endif // WEBGPU_BACKEND_WGPU

#if defined(WEBGPU_BACKEND_DAWN)
            device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
            wgpuDevicePoll(device, false, nullptr);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
            emscripten_sleep(100);
#endif
        }
    END:
        queue.release();
        device.release();
        surface.release();
        instance.release();
    }

    return 0;
}