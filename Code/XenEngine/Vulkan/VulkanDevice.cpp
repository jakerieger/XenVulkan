// Author: Jake Rieger
// Created: 1/9/2025.
//

#include "VulkanDevice.hpp"

#include "VulkanStruct.hpp"

#include <algorithm>
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
        auto bestDevice = SelectBestDevice(devices, surface);
        if (!bestDevice) { Panic("No suitable GPU found."); }
        _physicalDevice     = bestDevice;
        _queueFamilyIndices = FindQueueFamilies(bestDevice, surface);

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

    struct DeviceScore {
        VkPhysicalDevice device;
        i32 score;

        DeviceScore(VkPhysicalDevice d, const i32 s) : device(d), score(s) {}

        bool operator<(const DeviceScore& other) const {
            return score > other.score;  // > for descending order
        }
    };

    i32 VulkanDevice::ScorePhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface) {
        i32 score = 0;

        // Query device properties and features
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Score based on device type - this is our primary differentiator
        switch (deviceProperties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                score += 10000;  // Strongly prefer discrete GPUs
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                score += 1000;  // Integrated GPUs are second choice
                break;
            default:
                return 0;  // All other types (CPU, virtual, other) get no score
        }

        // Check for required features - these are mandatory
        if (!deviceFeatures.geometryShader) {
            return 0;  // Device is unsuitable if it lacks required features
        }

        // Check queue families
        QueueFamilyIndices indices = FindQueueFamilies(device, surface);
        if (!indices.IsComplete()) {
            return 0;  // Device is unsuitable if it lacks required queue families
        }

        // Check extension support
        u32 extensionCount;
        vkEnumerateDeviceExtensionProperties(device, None, &extensionCount, None);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device,
                                             None,
                                             &extensionCount,
                                             availableExtensions.data());

        auto required = GetRequiredDeviceExtensions();
        std::set<str> requiredExtensions(required.begin(), required.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        if (!requiredExtensions.empty()) {
            return 0;  // Device is unsuitable if it lacks required extensions
        }

        // Score based on device limits
        score +=
          deviceProperties.limits.maxImageDimension2D / 4096;  // Reward higher texture dimensions

        // Score based on memory size (if available)
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

        VkDeviceSize totalMemory = 0;
        for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
            if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                totalMemory += memProperties.memoryHeaps[i].size;
            }
        }
        score +=
          static_cast<int>(totalMemory / (1024 * 1024 * 1024));  // Add points per GB of memory

        return score;
    }

    VkPhysicalDevice VulkanDevice::SelectBestDevice(const std::vector<VkPhysicalDevice>& devices,
                                                    VkSurfaceKHR surface) {
        std::vector<DeviceScore> scoredDevices;
        scoredDevices.reserve(devices.size());
        for (const auto& device : devices) {
            i32 score = ScorePhysicalDevice(device, surface);
            if (score > 0) scoredDevices.emplace_back(device, score);
        }
        std::sort(scoredDevices.begin(), scoredDevices.end());
        return !scoredDevices.empty() ? scoredDevices[0].device : VK_NULL_HANDLE;
    }

    bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        return ScorePhysicalDevice(device, surface) > 0;
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