// Author: Jake Rieger
// Created: 1/9/2025.
//

#include "VulkanContext.hpp"
#include "VulkanStruct.hpp"
#include <GLFW/glfw3.h>

namespace x::vk {
    VulkanContext::VulkanContext(GLFWwindow** window) {
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
        // TODO: Enable validation layers

        if (vkCreateInstance(&createInfo, None, &_instance) != VK_SUCCESS) {
            Panic("Failed to create vulkan instance.");
        }

        if (glfwCreateWindowSurface(_instance, *window, None, &_surface) != VK_SUCCESS) {
            Panic("Failed to create window surface.");
        }

        _device = std::make_unique<VulkanDevice>();
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