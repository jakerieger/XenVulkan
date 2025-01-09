// Author: Jake Rieger
// Created: 1/9/2025.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "VulkanDevice.hpp"

namespace x::vk {
    class VulkanContext {
    public:
        explicit VulkanContext(GLFWwindow** window, bool enableValidationLayers = false);
        ~VulkanContext();

        [[nodiscard]] VkInstance GetInstance() const;
        [[nodiscard]] VkSurfaceKHR GetSurface() const;
        [[nodiscard]] VulkanDevice* GetDevice() const;

    private:
        VkInstance _instance = VK_NULL_HANDLE;
        VkSurfaceKHR _surface = VK_NULL_HANDLE;
        std::unique_ptr<VulkanDevice> _device;
    };
}  // namespace x::vk
