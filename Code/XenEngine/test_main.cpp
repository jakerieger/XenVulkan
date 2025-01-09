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
#include "Filesystem.hpp"
#include "Vulkan/VulkanStruct.hpp"

#include <vector>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <queue>

namespace x {
    using namespace Filesystem;

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

        GLFWwindow* _window              = None;
        VkInstance _instance             = VK_NULL_HANDLE;
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device                 = VK_NULL_HANDLE;
        VkQueue _graphicsQueue           = VK_NULL_HANDLE;
        VkSurfaceKHR _surface            = VK_NULL_HANDLE;
        VkQueue _presentQueue            = VK_NULL_HANDLE;
        VkSwapchainKHR _swapChain        = VK_NULL_HANDLE;
        VkFormat _swapChainFormat        = VK_FORMAT_UNDEFINED;
        VkExtent2D _swapChainExtent      = {0, 0};
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass _renderPass         = VK_NULL_HANDLE;
        VkPipeline _pipeline             = VK_NULL_HANDLE;
        VkCommandPool _commandPool       = VK_NULL_HANDLE;
        VkCommandBuffer _commandBuffer   = VK_NULL_HANDLE;
        std::vector<VkImage> _swapChainImages;
        std::vector<VkImageView> _swapChainViews;
        std::vector<VkFramebuffer> _frameBuffers;
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

        void RecordCommandBuffer(const VkCommandBuffer commandBuffer, const u32 imageIndex) const {
            vk::VulkanStruct<VkCommandBufferBeginInfo> beginInfo;

            // Note: If the command buffer was already recorded once, then a call to
            // vkBeginCommandBuffer will implicitly reset it. It's not possible to append commands
            // to a buffer at a later time.
            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                Panic("Failed to begin recording command buffer.");
            }

            constexpr VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            vk::VulkanStruct<VkRenderPassBeginInfo> renderPassInfo;
            renderPassInfo.renderPass        = _renderPass;
            renderPassInfo.framebuffer       = _frameBuffers[imageIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = _swapChainExtent;
            renderPassInfo.clearValueCount   = 1;
            renderPassInfo.pClearValues      = &clearColor;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

            VkViewport viewport {};
            viewport.x        = 0.0f;
            viewport.y        = 0.0f;
            viewport.width    = CAST<f32>(_swapChainExtent.width);
            viewport.height   = CAST<f32>(_swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor {};
            scissor.offset = {0, 0};
            scissor.extent = _swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffer);
            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
                Panic("Failed to record command buffer.");
            }
        }

        void CreateCommandBuffer() {
            vk::VulkanStruct<VkCommandBufferAllocateInfo> allocInfo;
            allocInfo.commandPool        = _commandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            if (vkAllocateCommandBuffers(_device, &allocInfo, &_commandBuffer) != VK_SUCCESS) {
                Panic("Failed to allocate command buffers.");
            }
            printf("Allocated (%d) command buffers.\n", 1U);
        }

        void CreateCommandPool() {
            const auto indices = FindQueueFamilies(_physicalDevice);
            vk::VulkanStruct<VkCommandPoolCreateInfo> poolInfo;
            poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = indices.Values().first;
            if (vkCreateCommandPool(_device, &poolInfo, None, &_commandPool) != VK_SUCCESS) {
                Panic("Failed to create command pool.");
            }
            printf("Created command pool.\n");
        }

        void CreateFramebuffers() {
            _frameBuffers.resize(_swapChainViews.size());
            for (size_t i = 0; i < _swapChainViews.size(); i++) {
                const VkImageView attachments[] = {_swapChainViews[i]};
                vk::VulkanStruct<VkFramebufferCreateInfo> framebufferInfo;
                framebufferInfo.renderPass      = _renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments    = attachments;
                framebufferInfo.width           = _swapChainExtent.width;
                framebufferInfo.height          = _swapChainExtent.height;
                framebufferInfo.layers          = 1;
                if (vkCreateFramebuffer(_device, &framebufferInfo, None, &_frameBuffers[i]) !=
                    VK_SUCCESS) {
                    Panic("Failed to create framebuffer for index %llu", i);
                }
            }
            printf("Created (%llu) framebuffers.\n", _frameBuffers.size());
        }

        void CreateRenderPass() {
            VkAttachmentDescription colorAttachment = {};
            colorAttachment.format                  = _swapChainFormat;
            colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment            = 0;
            colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &colorAttachmentRef;

            vk::VulkanStruct<VkRenderPassCreateInfo> renderPassInfo;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments    = &colorAttachment;
            renderPassInfo.subpassCount    = 1;
            renderPassInfo.pSubpasses      = &subpass;

            if (vkCreateRenderPass(_device, &renderPassInfo, None, &_renderPass) != VK_SUCCESS) {
                Panic("Failed to create render pass.");
            }

            std::cout << "Created render pass.\n";
        }

        VkShaderModule CreateShaderModule(const std::vector<u8>& bytecode) const {
            vk::VulkanStruct<VkShaderModuleCreateInfo> createInfo;
            createInfo.codeSize = bytecode.size();
            createInfo.pCode    = RCAST<const u32*>(
              bytecode.data());  // Has to be cast to const 32-bit unsigned for some reason?
            VkShaderModule module;
            if (vkCreateShaderModule(_device, &createInfo, None, &module) != VK_SUCCESS) {
                Panic("Failed to create shader module.");
            }
            return module;
        }

        void CreatePipeline() {
            const auto vertCode   = FileReader::ReadAllBytes("Shaders/Unlit.vert.spv");
            const auto fragCode   = FileReader::ReadAllBytes("Shaders/Unlit.frag.spv");
            const auto vertModule = CreateShaderModule(vertCode);
            const auto fragModule = CreateShaderModule(fragCode);

            vk::VulkanStruct<VkPipelineShaderStageCreateInfo> vertCreateInfo;
            vertCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            vertCreateInfo.module = vertModule;
            vertCreateInfo.pName  = "main";  // main entry point
            vertCreateInfo.pSpecializationInfo =
              None;  // Allows us to update constants at compile time for branch optimization

            vk::VulkanStruct<VkPipelineShaderStageCreateInfo> fragCreateInfo;
            fragCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragCreateInfo.module = fragModule;
            fragCreateInfo.pName  = "main";  // main entry point
            fragCreateInfo.pSpecializationInfo =
              None;  // Allows us to update constants at compile time for branch optimization

            VkPipelineShaderStageCreateInfo stages[] = {vertCreateInfo, fragCreateInfo};

            vk::VulkanStruct<VkPipelineVertexInputStateCreateInfo> vertexInputInfo;
            vertexInputInfo.vertexBindingDescriptionCount   = 0;
            vertexInputInfo.pVertexBindingDescriptions      = None;  // Optional
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.pVertexAttributeDescriptions    = None;  // Optional

            vk::VulkanStruct<VkPipelineInputAssemblyStateCreateInfo> inputAssembly;
            inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            // VkViewport viewport = {};
            // viewport.x          = 0.0f;
            // viewport.y          = 0.0f;
            // viewport.width      = CAST<f32>(_swapChainExtent.width);
            // viewport.height     = CAST<f32>(_swapChainExtent.height);
            // viewport.minDepth   = 0.0f;
            // viewport.maxDepth   = 1.0f;
            //
            // VkRect2D scissor = {};
            // scissor.offset   = {0, 0};
            // scissor.extent   = _swapChainExtent;

            std::vector<VkDynamicState> dynamicStates = {
              VK_DYNAMIC_STATE_VIEWPORT,
              VK_DYNAMIC_STATE_SCISSOR,
            };

            vk::VulkanStruct<VkPipelineDynamicStateCreateInfo> dynamicState;
            dynamicState.dynamicStateCount = CAST<u32>(dynamicStates.size());
            dynamicState.pDynamicStates    = dynamicStates.data();

            vk::VulkanStruct<VkPipelineViewportStateCreateInfo> viewportState;
            viewportState.viewportCount = 1;
            viewportState.scissorCount  = 1;

            vk::VulkanStruct<VkPipelineRasterizationStateCreateInfo> rasterizer;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.lineWidth        = 1.0f;
            rasterizer.cullMode         = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace        = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable  = VK_FALSE;

            // Using any mode other than FILL requires enabling a GPU feature. (sigh)
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

            // Enabling this blocks geometry from getting passed to the rasterizer.
            // Effecttively disables any output to the framebuffer.
            rasterizer.rasterizerDiscardEnable = VK_FALSE;

            vk::VulkanStruct<VkPipelineMultisampleStateCreateInfo> multisampling;
            multisampling.sampleShadingEnable  = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            // Enable alpha-blending by default
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
              VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable         = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

            vk::VulkanStruct<VkPipelineColorBlendStateCreateInfo> colorBlending;
            colorBlending.logicOpEnable = VK_FALSE;  // If enabled, uses bitwise combination
            colorBlending.attachmentCount =
              1;  // Must match the colorAttachmentCount in the subpass
            colorBlending.pAttachments =
              &colorBlendAttachment;  // Point to the attachment state you defined

            // Create an empty layout for now since none of our shaders require uniforms
            vk::VulkanStruct<VkPipelineLayoutCreateInfo> pipelineLayoutInfo;

            if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, None, &_pipelineLayout) !=
                VK_SUCCESS) {
                Panic("Failed to create pipeline layout.");
            }

            vk::VulkanStruct<VkGraphicsPipelineCreateInfo> pipelineInfo;
            pipelineInfo.stageCount          = 2;
            pipelineInfo.pStages             = stages;
            pipelineInfo.pVertexInputState   = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState      = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState   = &multisampling;
            pipelineInfo.pDepthStencilState  = None;  // Optional
            pipelineInfo.pColorBlendState    = &colorBlending;
            pipelineInfo.pDynamicState       = &dynamicState;
            pipelineInfo.layout              = _pipelineLayout;
            pipelineInfo.renderPass          = _renderPass;
            pipelineInfo.subpass             = 0;
            pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;  // Optional
            pipelineInfo.basePipelineIndex   = -1;              // Optional
            // pipelineInfo.flags               = VK_PIPELINE_CREATE_DERIVATIVE_BIT;

            if (vkCreateGraphicsPipelines(_device,
                                          VK_NULL_HANDLE,
                                          1,
                                          &pipelineInfo,
                                          None,
                                          &_pipeline) != VK_SUCCESS) {
                Panic("Failed to create pipeline.");
            }

            vkDestroyShaderModule(_device, vertModule, None);
            vkDestroyShaderModule(_device, fragModule, None);

            std::cout << "Created pipeline.\n";
        }

        void CreateImageViews() {
            _swapChainViews.resize(_swapChainImages.size());
            for (size_t i = 0; i < _swapChainImages.size(); i++) {
                vk::VulkanStruct<VkImageViewCreateInfo> createInfo;
                createInfo.image                           = _swapChainImages.at(i);
                createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format                          = _swapChainFormat;
                createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseMipLevel   = 0;
                createInfo.subresourceRange.levelCount     = 1;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount     = 1;
                if (vkCreateImageView(_device, &createInfo, None, &_swapChainViews[i]) !=
                    VK_SUCCESS) {
                    Panic("Failed to create image views.");
                }
            }

            printf("Created (%llu) image views.\n", _swapChainImages.size());
        }

        void CreateSwapChain() {
            const auto supportInfo   = QuerySwapChainSupport(_physicalDevice);
            const auto surfaceFormat = ChooseSwapChainFormat(supportInfo.formats);
            const auto presentMode   = ChooseSwapChainMode(supportInfo.presentModes);
            const auto extent        = ChooseSwapExtent(supportInfo.capabilities);
            u32 imageCount           = supportInfo.capabilities.minImageCount + 1;
            if (supportInfo.capabilities.maxImageCount > 0 &&
                imageCount > supportInfo.capabilities.maxImageCount) {
                imageCount = supportInfo.capabilities.maxImageCount;
            }
            const auto indices                  = FindQueueFamilies(_physicalDevice);
            const auto [graphics, presentation] = indices.Values();
            const u32 familyIndices[]           = {graphics, presentation};

            vk::VulkanStruct<VkSwapchainCreateInfoKHR> createInfo;
            createInfo.surface          = _surface;
            createInfo.minImageCount    = imageCount;
            createInfo.imageFormat      = surfaceFormat.format;
            createInfo.imageColorSpace  = surfaceFormat.colorSpace;
            createInfo.imageExtent      = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (graphics != presentation) {
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices   = familyIndices;
            } else {
                createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;     // Optional
                createInfo.pQueueFamilyIndices   = None;  // Optional
            }
            createInfo.preTransform   = supportInfo.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode    = presentMode;
            createInfo.clipped        = VK_TRUE;
            createInfo.oldSwapchain   = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(_device, &createInfo, None, &_swapChain) != VK_SUCCESS) {
                Panic("Failed to create swap chain.");
            }

            vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, None);
            _swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

            _swapChainFormat = surfaceFormat.format;
            _swapChainExtent = extent;

            std::cout << "Created swap chain.\n";
        }

        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
            if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
                return capabilities.currentExtent;
            }
            i32 width, height;
            glfwGetFramebufferSize(_window, &width, &height);
            VkExtent2D extent = {CAST<u32>(width), CAST<u32>(height)};
            extent.width      = std::clamp(extent.width,
                                      capabilities.minImageExtent.width,
                                      capabilities.maxImageExtent.width);
            extent.height     = std::clamp(extent.height,
                                       capabilities.minImageExtent.height,
                                       capabilities.maxImageExtent.height);
            return extent;
        }

        static VkPresentModeKHR
        ChooseSwapChainMode(const std::vector<VkPresentModeKHR>& availableModes) {
            for (const auto& mode : availableModes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) { return mode; }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        static VkSurfaceFormatKHR
        ChooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& info : availableFormats) {
                if (info.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    info.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return info;
                }
            }
            return availableFormats.at(0);
        }

        SwapChainSupportInfo QuerySwapChainSupport(const VkPhysicalDevice device) const {
            SwapChainSupportInfo info = {};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &info.capabilities);

            u32 fmtCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &fmtCount, None);
            if (fmtCount == 0) { Panic("No surface formats found for selected GPU and surface."); }
            info.formats.resize(fmtCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &fmtCount, info.formats.data());

            u32 presentCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentCount, None);
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
            vkEnumerateDeviceExtensionProperties(device, None, &extCount, None);
            std::vector<VkExtensionProperties> availableExtensions(extCount);
            vkEnumerateDeviceExtensionProperties(device,
                                                 None,
                                                 &extCount,
                                                 availableExtensions.data());
            std::set<str> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
            for (const auto& ext : availableExtensions) {
                requiredExtensions.erase(ext.extensionName);
            }
            return requiredExtensions.empty();
        }

        void CreateSurface() {
            if (glfwCreateWindowSurface(_instance, _window, None, &_surface) != VK_SUCCESS) {
                Panic("Failed to create window surface.");
            }
            std::cout << "Created window surface.\n";
        }

        void CreateLogicalDevice() {
            QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            const auto [graphics, present] = indices.Values();
            std::set uniqueQueueFamilies   = {graphics, present};
            f32 queuePriority              = 1.0f;
            for (u32 queueFamily : uniqueQueueFamilies) {
                vk::VulkanStruct<VkDeviceQueueCreateInfo> info;
                info.queueFamilyIndex = queueFamily;
                info.queueCount       = 1;
                info.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(info);
            }

            VkPhysicalDeviceFeatures deviceFeatures = {};

            vk::VulkanStruct<VkDeviceCreateInfo> createInfo;
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

            if (vkCreateDevice(_physicalDevice, &createInfo, None, &_device) != VK_SUCCESS) {
                Panic("Failed to create Vulkan logical device.");
            }

            vkGetDeviceQueue(_device, graphics, 0, &_graphicsQueue);
            vkGetDeviceQueue(_device, present, 0, &_presentQueue);

            std::cout
              << "Created logical device.\nCreated graphics queue.\nCreated presentation queue.\n";
        }

        QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice device) const {
            QueueFamilyIndices indices;

            u32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, None);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device,
                                                     &queueFamilyCount,
                                                     queueFamilies.data());
            for (i32 i = 0; i < CAST<i32>(queueFamilyCount); i++) {
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
            vkEnumeratePhysicalDevices(_instance, &deviceCount, None);
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
        }

        bool CheckValidationLayerSupport() const {
            u32 layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, None);
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

            vk::VulkanStruct<VkApplicationInfo> appInfo;
            appInfo.pApplicationName   = "XenVulkan";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName        = "Xen";
            appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
            appInfo.apiVersion         = VK_API_VERSION_1_0;

            vk::VulkanStruct<VkInstanceCreateInfo> createInfo;
            createInfo.pApplicationInfo = &appInfo;

            u32 glfwExtCount                   = 0;
            cstr* glfwExtensions               = glfwGetRequiredInstanceExtensions(&glfwExtCount);
            createInfo.enabledExtensionCount   = glfwExtCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;
            createInfo.enabledLayerCount       = 0;
            if (_enableValidationLayers) {
                createInfo.enabledLayerCount   = CAST<u32>(_validationLayers.size());
                createInfo.ppEnabledLayerNames = _validationLayers.data();
            }

            if (vkCreateInstance(&createInfo, None, &_instance) != VK_SUCCESS) {
                Panic("Failed to create Vulkan instance.");
            }

            std::cout << "Created Vulkan instance.\n";
        }

        void InitWindow() {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            _window = glfwCreateWindow(800, 600, "Vulkan Test", None, None);
            if (!_window) { Panic("Failed to create GLFW window."); }
        }

        void InitVulkan() {
            CreateInstance();
            CreateSurface();
            GetPhysicalDevice();
            CreateLogicalDevice();
            CreateSwapChain();
            CreateImageViews();
            CreateRenderPass();
            CreatePipeline();
            CreateFramebuffers();
            CreateCommandPool();
            CreateCommandBuffer();
        }

        void MainLoop() const {
            while (!glfwWindowShouldClose(_window)) {
                glfwPollEvents();
            }
        }

        void Cleanup() const {
            vkDestroyCommandPool(_device, _commandPool, None);

            for (const auto framebuffer : _frameBuffers) {
                vkDestroyFramebuffer(_device, framebuffer, None);
            }

            vkDestroyPipeline(_device, _pipeline, None);
            vkDestroyPipelineLayout(_device, _pipelineLayout, None);
            vkDestroyRenderPass(_device, _renderPass, None);

            for (const auto view : _swapChainViews) {
                vkDestroyImageView(_device, view, None);
            }

            vkDestroySwapchainKHR(_device, _swapChain, None);
            vkDestroySurfaceKHR(_instance, _surface, None);
            vkDestroyDevice(_device, None);
            vkDestroyInstance(_instance, None);
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