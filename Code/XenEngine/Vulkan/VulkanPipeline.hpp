// Author: Jake Rieger
// Created: 1/9/25.
//

#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Types.hpp"
#include "Panic.inl"

namespace x::vk {
    class VulkanPipeline {
        friend class VulkanPipelineBuilder;

    public:
        explicit VulkanPipeline(VkDevice device);

        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline&)            = delete;
        VulkanPipeline& operator=(const VulkanPipeline&) = delete;

        VulkanPipeline(VulkanPipeline&& other) noexcept;
        VulkanPipeline& operator=(VulkanPipeline&& other) noexcept;

        // Bind the pipeline to a command buffer
        void Bind(VkCommandBuffer commandBuffer,
                  VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

        // Getter for the internal pipeline handle
        [[nodiscard]] VkPipeline Handle() const;

    private:
        VkDevice _device;
        VkPipeline _pipeline;
        VkPipelineLayout _layout;

        void Cleanup();
    };
}  // namespace x::vk
