// Author: Jake Rieger
// Created: 1/8/2025.
//

#pragma once

#include <vulkan/vulkan_core.h>

namespace x::vk {
    template<typename T>
    struct VulkanTypeMap;

    // Application Info
    template<>
    struct VulkanTypeMap<VkApplicationInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    };

    // Instance Creation
    template<>
    struct VulkanTypeMap<VkInstanceCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    };

    // Device Queue Creation
    template<>
    struct VulkanTypeMap<VkDeviceQueueCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    };

    // Device Creation
    template<>
    struct VulkanTypeMap<VkDeviceCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    };

    // Swapchain Creation
    template<>
    struct VulkanTypeMap<VkSwapchainCreateInfoKHR> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    };

    // Image View Creation
    template<>
    struct VulkanTypeMap<VkImageViewCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    };

    // Shader Module Creation
    template<>
    struct VulkanTypeMap<VkShaderModuleCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    };

    // Pipeline Layout Creation
    template<>
    struct VulkanTypeMap<VkPipelineLayoutCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    };

    // Render Pass Creation
    template<>
    struct VulkanTypeMap<VkRenderPassCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    };

    // Graphics Pipeline Creation and its substates
    template<>
    struct VulkanTypeMap<VkGraphicsPipelineCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineShaderStageCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineVertexInputStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineInputAssemblyStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineViewportStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineRasterizationStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineMultisampleStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineColorBlendStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkFramebufferCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkCommandPoolCreateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkCommandBufferAllocateInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    };

    template<>
    struct VulkanTypeMap<VkCommandBufferBeginInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    };

    template<>
    struct VulkanTypeMap<VkRenderPassBeginInfo> {
        static constexpr VkStructureType value = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    };

    template<>
    struct VulkanTypeMap<VkPipelineDynamicStateCreateInfo> {
        static constexpr VkStructureType value =
          VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    };

    template<typename T>
    class VulkanStruct : public T {
    public:
        VulkanStruct() : T {} {
            this->sType = VulkanTypeMap<T>::value;
        }

        operator T&() {
            return *this;
        }

        operator const T&() const {
            return *this;
        }
    };
}  // namespace x::vk
