// Author: Jake Rieger
// Created: 1/9/2025.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include "Types.hpp"
#include "Panic.inl"

namespace x::vk {
    class VulkanDevice {
    public:
        VulkanDevice();
        ~VulkanDevice();

    private:
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device                 = VK_NULL_HANDLE;
    };
}  // namespace x::vk
