// Author: Jake Rieger
// Created: 1/9/2025.
//

#include "VulkanDevice.hpp"

#include "VulkanStruct.hpp"

#include <set>

namespace x::vk {
    VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface) {
        SelectPhysicalDevice(instance, surface);
        CreateLogicalDevice();
    }

    VulkanDevice::~VulkanDevice() {
        if (_device != None) vkDestroyDevice(_device, None);
    }

    void VulkanDevice::SelectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, None);

        if (deviceCount == 0) { Panic("Failed to find GPUs with Vulkan support"); }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Find first suitable device that meets our requirements
        for (const auto& device : devices) {
            if (IsDeviceSuitable(device, surface)) {
                _physicalDevice     = device;
                _queueFamilyIndices = FindQueueFamilies(device, surface);
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) { Panic("Failed to find a suitable GPU"); }
        printf("Found usable GPU\n");
    }

    void VulkanDevice::CreateLogicalDevice() {
        // Gather unique queue families needed for our device. Using a set ensures
        // we only create each queue family once, even if graphics and present
        // queues end up being from the same family
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<u32> uniqueQueueFamilies = {_queueFamilyIndices.graphicsFamily.value(),
                                             _queueFamilyIndices.presentFamily.value()};

        // Queue priority determines scheduling behavior. A value of 1.0 gives
        // our queues the highest priority possible
        f32 queuePriority = 1.0f;
        for (u32 queueFamily : uniqueQueueFamilies) {
            VulkanStruct<VkDeviceQueueCreateInfo> queueCreateInfo;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount       = 1;  // We only need one queue per family for now
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Specify which device features we'll be using. This is important because
        // requesting features the device doesn't support will cause device creation to fail
        VkPhysicalDeviceFeatures deviceFeatures {};
        deviceFeatures.geometryShader = VK_TRUE;  // We'll need this for advanced rendering

        // Main device creation info structure
        VulkanStruct<VkDeviceCreateInfo> createInfo;
        createInfo.queueCreateInfoCount = CAST<u32>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos    = queueCreateInfos.data();
        createInfo.pEnabledFeatures     = &deviceFeatures;

        // Add required device extensions - this is crucial for presenting to the window surface
        auto extensions                    = GetRequiredDeviceExtensions();
        createInfo.enabledExtensionCount   = CAST<u32>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Create the logical device - this is our main interface to the GPU
        if (vkCreateDevice(_physicalDevice, &createInfo, None, &_device) != VK_SUCCESS) {
            Panic("Failed to create logical device");
        }

        // Retrieve queue handles for later use. Index 0 is used since we only created
        // one queue per family
        vkGetDeviceQueue(_device, _queueFamilyIndices.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, _queueFamilyIndices.presentFamily.value(), 0, &_presentQueue);
    }

    bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        // Query device properties for making informed decisions about device selection
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Check for required queue families
        QueueFamilyIndices indices = FindQueueFamilies(device, surface);

        // Verify extension support
        u32 extensionCount;
        vkEnumerateDeviceExtensionProperties(device, None, &extensionCount, None);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device,
                                             None,
                                             &extensionCount,
                                             availableExtensions.data());

        // Create a set of required extensions for easy lookup
        auto required = GetRequiredDeviceExtensions();
        std::set<str> requiredExtensions(required.begin(), required.end());

        // Remove extensions we find from our required set
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        // Device is considered suitable if:
        // 1. It's a discrete GPU (better performance than integrated) or worst case, an integrated
        // GPU
        // 2. It supports geometry shaders
        // 3. It has all required queue families
        // 4. It supports all required extensions (set is empty after finding them all)
        return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
                deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
               deviceFeatures.geometryShader && indices.IsComplete() && requiredExtensions.empty();
    }

    QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice device,
                                                       VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        // Query queue family properties
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, None);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // Iterate through queue families to find ones supporting our required operations
        for (u32 i = 0; i < queueFamilies.size(); i++) {
            // Check for graphics support
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.graphicsFamily = i; }

            // Check for presentation support
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) { indices.presentFamily = i; }

            if (indices.IsComplete()) { break; }
        }

        return indices;
    }

    std::vector<cstr> VulkanDevice::GetRequiredDeviceExtensions() {
        return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    }
}  // namespace x::vk