// Author: Jake Rieger
// Created: 1/9/2025.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include "Types.hpp"
#include "Panic.inl"

namespace x::vk {
    struct QueueFamilyIndices {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;
        std::optional<u32> computeFamily;

        [[nodiscard]] bool IsComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value() &&
                   computeFamily.has_value();
        }
    };

    class VulkanDevice {
    public:
        VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
        ~VulkanDevice();

        VulkanDevice(const VulkanDevice&)            = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;

        // Utility functions
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const {
            return _physicalDevice;
        }
        [[nodiscard]] VkDevice GetLogicalDevice() const {
            return _device;
        }
        [[nodiscard]] const QueueFamilyIndices& GetQueueFamilyIndices() const {
            return _queueFamilyIndices;
        }

    private:
        void SelectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
        void CreateLogicalDevice();

        [[nodiscard]] static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
        [[nodiscard]] static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
                                                                  VkSurfaceKHR surface);
        [[nodiscard]] static std::vector<const char*> GetRequiredDeviceExtensions();
        [[nodiscard]] static i32 ScorePhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
        static VkPhysicalDevice SelectBestDevice(const std::vector<VkPhysicalDevice>& devices,
                                                 VkSurfaceKHR surface);

        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device                 = VK_NULL_HANDLE;
        VkQueue _graphicsQueue           = VK_NULL_HANDLE;
        VkQueue _presentQueue            = VK_NULL_HANDLE;
        VkQueue _computeQueue            = VK_NULL_HANDLE;
        QueueFamilyIndices _queueFamilyIndices;
    };
}  // namespace x::vk
