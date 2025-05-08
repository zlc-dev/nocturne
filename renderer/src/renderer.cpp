/*
    renderer.cpp
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

#include "renderer.h"
#include <iostream>
#include <webgpu/webgpu.hpp>

void inspectAdapter(wgpu::Adapter adapter) {
    wgpu::SupportedLimits supported_limits = {};
    supported_limits.nextInChain = nullptr;
    #ifdef WEBGPU_BACKEND_DAWN
    bool success = adapter.getLimits(&supported_limits) == WGPUStatus_Success;
    #else 
    bool success = adapter.getLimits(&supported_limits);
    #endif
    if(!success) {
        std::cout << "Cannot get Adapter limits\n";
    } else {
        std::cout << "Adapter limits:\n";
        std::cout << " - maxTextureDimension1D: " << supported_limits.limits.maxTextureDimension1D << '\n';
        std::cout << " - maxTextureDimension2D: " << supported_limits.limits.maxTextureDimension2D << '\n';
        std::cout << " - maxTextureDimension3D: " << supported_limits.limits.maxTextureDimension3D << '\n';
        std::cout << " - maxTextureArrayLayers: " << supported_limits.limits.maxTextureArrayLayers << '\n';
        std::cout << " - maxVertexAttributes:   " << supported_limits.limits.maxVertexAttributes   << '\n'; 
    }
    wgpu::AdapterProperties properties = {};
    properties.nextInChain = nullptr;
    #ifdef WEBGPU_BACKEND_DAWN
    success = adapter.getProperties(&properties) == WGPUStatus_Success;
    #else 
    adapter.getProperties(&properties);
    success = true;
    #endif
    if(!success) {
        std::cout << "Cannot get Adapter properties\n";
    } else {
        std::cout << "Adapter properties:\n";
        std::cout << " - vendorID: " << properties.vendorID << '\n';
        if (properties.vendorName) {
            std::cout << " - vendorName: " << properties.vendorName << '\n';
        }
        if (properties.architecture) {
            std::cout << " - architecture: " << properties.architecture << '\n';
        }
        std::cout << " - deviceID: " << properties.deviceID << '\n';
        if (properties.name) {
            std::cout << " - name: " << properties.name << '\n';
        }
        if (properties.driverDescription) {
            std::cout << " - driverDescription: " << properties.driverDescription << '\n';
        }
        std::cout << std::hex;
        std::cout << " - adapterType: 0x" << properties.adapterType << '\n';
        std::cout << " - backendType: 0x" << properties.backendType << '\n';
        std::cout << std::dec;
    }
}

void inspectDevice(wgpu::Device device) {
    std::vector<wgpu::FeatureName> features;
    size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
    features.resize(featureCount, wgpu::FeatureName::Undefined);
    device.enumerateFeatures(features.data());

    std::cout << "Device features:\n";
    std::cout << std::hex;
    for (auto f : features) {
        std::cout << " - 0x" << f << '\n';
    }
    std::cout << std::dec;

    WGPUSupportedLimits limits = {};
    limits.nextInChain = nullptr;

#ifdef WEBGPU_BACKEND_DAWN
    bool success = wgpuDeviceGetLimits(device, &limits) == WGPUStatus_Success;
#else
    bool success = wgpuDeviceGetLimits(device, &limits);
#endif

    if (success) {
        std::cout << "Device limits:\n";
        std::cout << " - maxTextureDimension1D: " << limits.limits.maxTextureDimension1D << '\n';
        std::cout << " - maxTextureDimension2D: " << limits.limits.maxTextureDimension2D << '\n';
        std::cout << " - maxTextureDimension3D: " << limits.limits.maxTextureDimension3D << '\n';
        std::cout << " - maxTextureArrayLayers: " << limits.limits.maxTextureArrayLayers << '\n';
        std::cout << " - maxVertexAttributes:   " << limits.limits.maxVertexAttributes   << '\n'; 
        std::cout << " - maxVertexBuffers:      " << limits.limits.maxVertexBuffers      << '\n'; 
    }
}

int test() {
    std::cout << "renderer hello\n";

    // We create a descriptor
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

    // We create the instance using this descriptor
    WGPUInstance instance = wgpuCreateInstance(&desc);

    // We can check whether there is actually an instance created
    if (!instance) {
        std::cerr << "Could not initialize WebGPU!\n";
        return 1;
    }

    // Display the object (WGPUInstance is a simple pointer, it may be
    // copied around without worrying about its size).
    std::cout << "WGPU instance: " << instance << '\n';

    return 0;
}