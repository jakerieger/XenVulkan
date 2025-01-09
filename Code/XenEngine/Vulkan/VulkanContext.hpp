// Author: Jake Rieger
// Created: 1/9/2025.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "VulkanDevice.hpp"

namespace x::vk {
    class SwapChain;
    class RenderPass;
    class Pipeline;

    class VulkanContext {
    public:
        explicit VulkanContext(GLFWwindow** window, bool enableValidationLayers = false);
        ~VulkanContext();

        VkInstance GetInstance() const;
        VkSurfaceKHR GetSurface() const;
        VulkanDevice* GetDevice() const;

    private:
        VkInstance _instance;
        VkSurfaceKHR _surface;
        std::unique_ptr<VulkanDevice> _device;
        // std::unique_ptr<SwapChain> _swapChain;
        // std::unique_ptr<RenderPass> _renderPass;
        // std::unique_ptr<Pipeline> _pipeline;
    };
}  // namespace x::vk
