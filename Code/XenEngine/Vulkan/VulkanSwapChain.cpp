// Author: Jake Rieger
// Created: 1/11/2025.
//

#include "VulkanSwapChain.hpp"
#include "VulkanStruct.hpp"
#include <algorithm>
#include "Panic.inl"

namespace x::vk {
    VulkanSwapChain::VulkanSwapChain(VulkanDevice* device,
                                     VkSurfaceKHR surface,
                                     u32 width,
                                     u32 height)
        : _device(device), _surface(surface) {
        CreateSwapChain(width, height);
    }

    VulkanSwapChain::~VulkanSwapChain() {
        Cleanup();
    }

    void VulkanSwapChain::Recreate(u32 width, u32 height) {
        Cleanup();
        CreateSwapChain(width, height);
    }

    void VulkanSwapChain::CreateSwapChain(u32 width, u32 height) {
        SwapChainSupportDetails swapSupport =
          QuerySwapChainSupport(_device->GetPhysicalDevice(), _surface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapSupport.formats);
        VkPresentModeKHR presentMode     = ChooseSwapPresentMode(swapSupport.presentModes);
        VkExtent2D extent = ChooseSwapExtent(swapSupport.capabilities, width, height);

        u32 imageCount = swapSupport.capabilities.minImageCount + 1;
        if (swapSupport.capabilities.maxImageCount > 0) {
            imageCount = std::min(imageCount, swapSupport.capabilities.maxImageCount);
        }

        VulkanStruct<VkSwapchainCreateInfoKHR> createInfo;
        createInfo.surface          = _surface;
        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;  // Always 1 unless developing a stereoscopic 3D app
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const QueueFamilyIndices indices = _device->GetQueueFamilyIndices();
        if (indices.graphicsFamily != indices.presentFamily) {
            // If graphics and present queues are different, we need concurrent sharing
            u32 queueFamilyIndices[]         = {indices.graphicsFamily.value(),
                                                indices.presentFamily.value()};
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform   = swapSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;  // Enable clipping for better performance
        createInfo.oldSwapchain   = None;

        if (vkCreateSwapchainKHR(_device->GetLogicalDevice(), &createInfo, None, &_swapChain) !=
            VK_SUCCESS) {
            Panic("Failed to create swap chain.");
        }

        u32 actualImageCount;
        vkGetSwapchainImagesKHR(_device->GetLogicalDevice(), _swapChain, &actualImageCount, None);
        _images.resize(actualImageCount);
        vkGetSwapchainImagesKHR(_device->GetLogicalDevice(),
                                _swapChain,
                                &actualImageCount,
                                _images.data());
        _imageFormat = surfaceFormat.format;
        _extent      = extent;

        CreateImageViews();
    }

    void VulkanSwapChain::CreateImageViews() {
        _imageViews.resize(_images.size());

        for (size_t i = 0; i < _images.size(); i++) {
            VulkanStruct<VkImageViewCreateInfo> createInfo;
            createInfo.image                           = _images.at(i);
            createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format                          = _imageFormat;
            createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            if (vkCreateImageView(_device->GetLogicalDevice(),
                                  &createInfo,
                                  None,
                                  &_imageViews.at(i)) != VK_SUCCESS) {
                Panic("Failed to create image views.");
            }
        }
    }

    void VulkanSwapChain::Cleanup() {
        for (const auto view : _imageViews) {
            vkDestroyImageView(_device->GetLogicalDevice(), view, None);
        }
        _imageViews.clear();

        if (_swapChain != None) {
            vkDestroySwapchainKHR(_device->GetLogicalDevice(), _swapChain, None);
            _swapChain = None;
        }
    }

    SwapChainSupportDetails VulkanSwapChain::QuerySwapChainSupport(VkPhysicalDevice device,
                                                                   VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        u32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, None);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                                 surface,
                                                 &formatCount,
                                                 details.formats.data());
        }

        u32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, None);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, None);
        }

        return details;
    }

    VkSurfaceFormatKHR
    VulkanSwapChain::ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& format : availableFormats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return availableFormats.at(0);
    }

    VkPresentModeKHR
    VulkanSwapChain::ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availableModes) {
        for (const auto& mode : availableModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) { return mode; }
        }

        // Fallback FIFO mode if triple buffering isn't available
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                                 u32 width,
                                                 u32 height) {
        if (capabilities.currentExtent.width != UINT64_MAX) { return capabilities.currentExtent; }

        VkExtent2D actualExtent = {width, height};
        actualExtent.width      = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height     = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }
}  // namespace x::vk