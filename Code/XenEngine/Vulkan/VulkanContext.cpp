// Author: Jake Rieger
// Created: 1/9/2025.
//

#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanContext.hpp"
#include "VulkanStruct.hpp"
#include "Types.hpp"
#include "Panic.inl"

namespace x::vk {
    static const std::vector kValidationLayers = {"VK_LAYER_KHRONOS_validation"};

    static bool CheckValidationLayerSupport() {
        u32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, None);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const auto name : kValidationLayers) {
            bool found = false;
            for (const auto& props : availableLayers) {
                if (strcmp(name, props.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    VulkanContext::VulkanContext(GLFWwindow** window, const bool enableValidationLayers) {
        bool validationAvailable = false;
        if (enableValidationLayers) {
            validationAvailable = CheckValidationLayerSupport();
            if (validationAvailable) {
                printf("Validation layers are supported.\n");
            } else {
                printf("Note: Validation layers were requested but are not available. "
                       "This is normal if you haven't installed vulkan-validationlayers. "
                       "Continuing without validation.\n");
            }
        }

        VulkanStruct<VkApplicationInfo> appInfo;
        appInfo.pApplicationName   = "Xen Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = "Xen";
        appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion         = VK_API_VERSION_1_0;

        VulkanStruct<VkInstanceCreateInfo> createInfo;
        createInfo.pApplicationInfo        = &appInfo;
        u32 extensionCount                 = 0;
        cstr* extensions                   = glfwGetRequiredInstanceExtensions(&extensionCount);
        createInfo.enabledExtensionCount   = extensionCount;
        createInfo.ppEnabledExtensionNames = extensions;
        createInfo.enabledLayerCount       = 0;
        if (enableValidationLayers && validationAvailable) {
            createInfo.enabledLayerCount   = CAST<u32>(kValidationLayers.size());
            createInfo.ppEnabledLayerNames = kValidationLayers.data();
        }

        if (vkCreateInstance(&createInfo, None, &_instance) != VK_SUCCESS) {
            Panic("Failed to create vulkan instance.");
        }

        if (glfwCreateWindowSurface(_instance, *window, None, &_surface) != VK_SUCCESS) {
            Panic("Failed to create window surface.");
        }

       _device = std::make_unique<VulkanDevice>(_instance, _surface);
    }

    VulkanContext::~VulkanContext() {
        _device.reset();
        vkDestroySurfaceKHR(_instance, _surface, None);
        vkDestroyInstance(_instance, None);
    }

    VkInstance VulkanContext::GetInstance() const {
        return _instance;
    }

    VkSurfaceKHR VulkanContext::GetSurface() const {
        return _surface;
    }

    VulkanDevice* VulkanContext::GetDevice() const {
        return _device.get();
    }
}  // namespace x::vk