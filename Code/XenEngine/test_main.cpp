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
#include <set>

namespace x {
    class TestApp {
    public:
        void Run() {
            InitWindow();
            InitVulkan();
            MainLoop();
            Cleanup();
        }

    private:
#ifdef NDEBUG
        const bool _enableValidationLayers = false;
#else
        const bool _enableValidationLayers = true;
#endif

        GLFWwindow* _window                       = nullptr;
        VkInstance _instance                      = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice          = VK_NULL_HANDLE;
        VkDevice _device                          = VK_NULL_HANDLE;
        VkQueue _graphicsQueue                    = VK_NULL_HANDLE;
        VkSurfaceKHR _surface                     = VK_NULL_HANDLE;
        VkQueue _presentQueue                     = VK_NULL_HANDLE;
        const std::vector<cstr> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<cstr> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        struct QueueFamilyIndices {
            std::optional<u32> graphicsFamily;
            std::optional<u32> presentFamily;

            bool Valid() const {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }

            std::pair<u32, u32> Values() const {
                return {graphicsFamily.value(), presentFamily.value()};
            }
        };

        struct SwapChainSupportInfo {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapChainSupportInfo QuerySwapChainSupport(const VkPhysicalDevice device) const {
            SwapChainSupportInfo info = {};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &info.capabilities);

            u32 fmtCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &fmtCount, nullptr);
            if (fmtCount == 0) { Panic("No surface formats found for selected GPU and surface."); }
            info.formats.resize(fmtCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &fmtCount, info.formats.data());

            u32 presentCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentCount, nullptr);
            if (presentCount == 0) {
                Panic("No presentation modes found for selected GPU and surface.");
            }
            info.presentModes.resize(presentCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                                      _surface,
                                                      &presentCount,
                                                      info.presentModes.data());

            return info;
        }

        // Checks for required extension support
        bool CheckDeviceExtSupport(const VkPhysicalDevice device) const {
            u32 extCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extCount);
            vkEnumerateDeviceExtensionProperties(device,
                                                 nullptr,
                                                 &extCount,
                                                 availableExtensions.data());
            std::set<str> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
            for (const auto& ext : availableExtensions) {
                requiredExtensions.erase(ext.extensionName);
            }
            return requiredExtensions.empty();
        }

        void CreateSurface() {
            if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
                Panic("Failed to create window surface.");
            }
            std::cout << "Window surface created.\n";
        }

        void CreateLogicalDevice() {
            QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            const auto [graphics, present] = indices.Values();
            std::set uniqueQueueFamilies   = {graphics, present};
            f32 queuePriority              = 1.0f;
            for (u32 queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo info = {};
                info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                info.queueFamilyIndex        = queueFamily;
                info.queueCount              = 1;
                info.pQueuePriorities        = &queuePriority;
                queueCreateInfos.push_back(info);
            }

            VkPhysicalDeviceFeatures deviceFeatures = {};

            VkDeviceCreateInfo createInfo      = {};
            createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos       = queueCreateInfos.data();
            createInfo.queueCreateInfoCount    = CAST<u32>(queueCreateInfos.size());
            createInfo.pEnabledFeatures        = &deviceFeatures;
            createInfo.enabledExtensionCount   = CAST<u32>(_deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
            if (_enableValidationLayers) {
                createInfo.enabledLayerCount   = CAST<u32>(_validationLayers.size());
                createInfo.ppEnabledLayerNames = _validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
                Panic("Failed to create Vulkan logical device.");
            }

            vkGetDeviceQueue(_device, graphics, 0, &_graphicsQueue);
            vkGetDeviceQueue(_device, present, 0, &_presentQueue);

            std::cout
              << "Logical device created.\nGraphics queue created.\nPresentation queue created.\n";
        }

        QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice device) const {
            QueueFamilyIndices indices;

            u32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device,
                                                     &queueFamilyCount,
                                                     queueFamilies.data());
            for (i32 i = 0; i < queueFamilyCount; i++) {
                if (const auto family = queueFamilies.at(i);
                    family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
                if (presentSupport) { indices.presentFamily = i; }

                if (indices.Valid()) break;
            }

            return indices;
        }

        bool IsDeviceSuitable(const VkPhysicalDevice device) const {
            VkPhysicalDeviceProperties deviceProps;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProps);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            const QueueFamilyIndices indices = FindQueueFamilies(device);
            const bool extensionsSupported   = CheckDeviceExtSupport(device);
            bool swapChainValid              = false;
            if (extensionsSupported) {
                SwapChainSupportInfo info = QuerySwapChainSupport(device);
                swapChainValid            = !info.formats.empty() && !info.presentModes.empty();
                // TODO: Add validation for HDR formats
            }
            return deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                   deviceFeatures.geometryShader && indices.Valid() && extensionsSupported &&
                   swapChainValid;
        }

        // Queries for and selects an appropriate GPU (if one exists)
        void GetPhysicalDevice() {
            u32 deviceCount = 0;
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
            u32 layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const cstr layerName : _validationLayers) {
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
                if (CheckValidationLayerSupport()) {
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

            u32 glfwExtCount                   = 0;
            cstr* glfwExtensions               = glfwGetRequiredInstanceExtensions(&glfwExtCount);
            createInfo.enabledExtensionCount   = glfwExtCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;
            createInfo.enabledLayerCount       = 0;
            if (_enableValidationLayers) {
                createInfo.enabledLayerCount   = CAST<u32>(_validationLayers.size());
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
            CreateSurface();
            GetPhysicalDevice();
            CreateLogicalDevice();
        }

        void MainLoop() const {
            while (!glfwWindowShouldClose(_window)) {
                glfwPollEvents();
            }
        }

        void Cleanup() const {
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            vkDestroyDevice(_device, nullptr);
            vkDestroyInstance(_instance, nullptr);
            glfwDestroyWindow(_window);
            glfwTerminate();
        }
    };
}  // namespace x

int main() {
    x::TestApp app;
    app.Run();
    return EXIT_SUCCESS;
}