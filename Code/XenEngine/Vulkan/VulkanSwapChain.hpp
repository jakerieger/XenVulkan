// Author: Jake Rieger
// Created: 1/11/2025.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include "Types.hpp"
#include "VulkanDevice.hpp"

namespace x::vk {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        vector<VkSurfaceFormatKHR> formats;
        vector<VkPresentModeKHR> presentModes;
    };

    class VulkanSwapChain {
    public:
        VulkanSwapChain(VulkanDevice* device, VkSurfaceKHR surface, u32 width, u32 height);
        ~VulkanSwapChain();

        // Prevent copying to avoid double-free of Vulkan resources
        VulkanSwapChain(const VulkanSwapChain&)            = delete;
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;

        // Allow moving for container storage and return values
        VulkanSwapChain(VulkanSwapChain&&) noexcept            = default;
        VulkanSwapChain& operator=(VulkanSwapChain&&) noexcept = default;

        void Recreate(u32 width, u32 height);

        [[nodiscard]] VkFormat GetImageFormat() const {
            return _imageFormat;
        }
        [[nodiscard]] VkExtent2D GetExtent() const {
            return _extent;
        }
        [[nodiscard]] const std::vector<VkImageView>& GetImageViews() const {
            return _imageViews;
        }
        [[nodiscard]] VkSwapchainKHR GetSwapchain() const {
            return _swapChain;
        }

    private:
        void CreateSwapChain(u32 width, u32 height);
        void CreateImageViews();
        void Cleanup();

        [[nodiscard]] static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
                                                                           VkSurfaceKHR surface);
        [[nodiscard]] static VkSurfaceFormatKHR
        ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
        [[nodiscard]] static VkPresentModeKHR
        ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availableModes);
        [[nodiscard]] static VkExtent2D
        ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, u32 width, u32 height);

        VulkanDevice* _device;
        VkSurfaceKHR _surface;
        VkSwapchainKHR _swapChain = None;
        vector<VkImage> _images;
        vector<VkImageView> _imageViews;
        VkFormat _imageFormat;
        VkExtent2D _extent;
    };
}  // namespace x::vk
