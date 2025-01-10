// Author: Jake Rieger
// Created: 1/9/25.
//

#include "VulkanPipeline.hpp"

namespace x::vk {
    VulkanPipeline::VulkanPipeline(VkDevice device)
        : _device(device), _pipeline(None), _layout(None) {}

    VulkanPipeline::~VulkanPipeline() {
        Cleanup();
    }

    VkPipeline VulkanPipeline::Handle() const {
        return _pipeline;
    }

    void VulkanPipeline::Cleanup() {
        if (_device) {
            if (_pipeline) {
                vkDestroyPipeline(_device, _pipeline, None);
                _pipeline = None;
            }
            if (_layout) {
                vkDestroyPipelineLayout(_device, _layout, None);
                _layout = None;
            }
        }
    }

    VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) noexcept
        : _device(other._device), _pipeline(other._pipeline), _layout(other._layout) {
        other._device   = None;
        other._pipeline = None;
        other._layout   = None;
    }

    VulkanPipeline& VulkanPipeline::operator=(VulkanPipeline&& other) noexcept {
        if (this != &other) {
            Cleanup();
            _device         = other._device;
            _pipeline       = other._pipeline;
            _layout         = other._layout;
            other._device   = None;
            other._pipeline = None;
            other._layout   = None;
        }
        return *this;
    }

    void VulkanPipeline::Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint) const {
        if (commandBuffer && _pipeline) { vkCmdBindPipeline(commandBuffer, bindPoint, _pipeline); }
    }
}  // namespace x::vk