// Author: Jake Rieger
// Created: 1/7/2025.
//

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Types.hpp"
#include "Panic.inl"

#include <vector>
#include <optional>

class TestApp {
public:
    void Run() {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    GLFWwindow* _window              = nullptr;
    VkInstance _instance             = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device                 = VK_NULL_HANDLE;
    VkQueue _graphicsQueue           = VK_NULL_HANDLE;

    const std::vector<x::cstr> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef NDEBUG
    const bool _enableValidationLayers = false;
#else
    const bool _enableValidationLayers = true;
#endif

    struct QueueFamilyIndices {
        std::optional<x::u32> graphicsFamily;

        bool Valid() const {
            return graphicsFamily.has_value();
        }

        x::u32 Value() const {
            return graphicsFamily.value();
        }
    };

    void CreateLogicalDevice() {
        QueueFamilyIndices indices          = FindQueueFamilies(_physicalDevice);
        VkDeviceQueueCreateInfo qCreateInfo = {};
        qCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qCreateInfo.queueFamilyIndex        = indices.Value();
        qCreateInfo.queueCount              = 1;
        constexpr x::f32 queuePriority      = 1.0f;
        qCreateInfo.pQueuePriorities        = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo    = {};
        createInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos     = &qCreateInfo;
        createInfo.queueCreateInfoCount  = 1;
        createInfo.pEnabledFeatures      = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;
        if (_enableValidationLayers) {
            createInfo.enabledLayerCount   = CAST<x::u32>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
            Panic("Failed to create Vulkan logical device.");
        }

        vkGetDeviceQueue(_device, indices.Value(), 0, &_graphicsQueue);
    }

    static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        x::u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        for (x::i32 i = 0; i < queueFamilyCount; i++) {
            if (indices.Valid()) break;
            if (const auto family = queueFamilies.at(i);
                family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
        }

        return indices;
    }

    static bool IsDeviceSuitable(const VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProps;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProps);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        const QueueFamilyIndices indices = FindQueueFamilies(device);
        return deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
               deviceFeatures.geometryShader && indices.Valid();
    }

    // Queries for and selects an appropriate GPU (if one exists)
    void GetPhysicalDevice() {
        x::u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
        if (!deviceCount) { Panic("Failed to find a Vulkan-compatible GPU."); }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (IsDeviceSuitable(device)) {
                _physicalDevice = device;
                break;
            }
        }

        if (_physicalDevice == VK_NULL_HANDLE) { Panic("Failed to find a suitable GPU."); }
        std::cout << "Found suitable GPU.\n";
    }

    bool CheckValidationLayerSupport() const {
        x::u32 layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const x::cstr layerName : _validationLayers) {
            bool layerFound = false;
            for (const auto& props : availableLayers) {
                if (strcmp(layerName, props.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) { return false; }
        }

        return true;
    }

    void CreateInstance() {
        if (_enableValidationLayers) {
            if (const auto result = CheckValidationLayerSupport()) {
                std::cout << "Validation layers enabled.\n";
            } else {
                Panic("Failed to enable validation layers for debug build.");
            }
        }

        VkApplicationInfo appInfo  = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName   = "XenVulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = "Xen";
        appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion         = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo     = &appInfo;

        x::u32 glfwExtCount                = 0;
        x::cstr* glfwExtensions            = glfwGetRequiredInstanceExtensions(&glfwExtCount);
        createInfo.enabledExtensionCount   = glfwExtCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount       = 0;
        if (_enableValidationLayers) {
            createInfo.enabledLayerCount   = CAST<x::u32>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        }

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
            Panic("Failed to create Vulkan instance.");
        }

        std::cout << "Vulkan instance created.\n";
    }

    void InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(800, 600, "Vulkan Test", nullptr, nullptr);
        if (!_window) { Panic("Failed to create GLFW window."); }
    }

    void InitVulkan() {
        CreateInstance();
        GetPhysicalDevice();
        CreateLogicalDevice();
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();
        }
    }

    void Cleanup() {
        vkDestroyDevice(_device, nullptr);
        vkDestroyInstance(_instance, nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }
};

int main() {
    TestApp app;
    app.Run();
    return EXIT_SUCCESS;
}