// Author: Jake Rieger
// Created: 1/9/25.
//

#include "VulkanPipelineBuilder.hpp"

namespace x::vk {
    VulkanPipelineBuilder::VulkanPipelineBuilder()
        : _layout(None), _renderPass(None), _subpass(0), _dynamicViewportAndScissor(false) {
        InitializeDefaults();
    }

    // TODO: Ignore this for now, I need to rework my shader system
    VulkanPipelineBuilder& VulkanPipelineBuilder::AddShaderStage(VkShaderStageFlagBits stage,
                                                                 const VulkanShader& shader) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetVertexInput(
      const vector<VkVertexInputBindingDescription>& bindings,
      const vector<VkVertexInputAttributeDescription>& attributes) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetInputAssembly(VkPrimitiveTopology topology,
                                                                   VkBool32 primitiveRestart) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetViewport(const VkViewport& viewport,
                                                              const VkRect2D& scissor) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetDynamicViewportAndScissor(u32 viewportCount) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetRasterizer(VkPolygonMode polygonMode,
                                                                VkCullModeFlags cullMode,
                                                                VkFrontFace frontFace,
                                                                f32 lineWidth) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetMultisampling(VkSampleCountFlagBits samples) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetDepthStencil(bool depthTest,
                                                                  bool depthWrite,
                                                                  VkCompareOp depthCompareOp) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetColorBlending(
      bool enableBlending, const vector<VkPipelineColorBlendAttachmentState>& attachments) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetPipelineLayout(VkPipelineLayout layout) {
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetRenderPass(VkRenderPass renderPass,
                                                                u32 subpass) {
        return *this;
    }

    VulkanPipeline VulkanPipelineBuilder::Build(VkDevice device) {
        auto pipeline = VulkanPipeline(device);

        // Setup pipeline configuration using builder settings

        return pipeline;
    }

    void VulkanPipelineBuilder::InitializeDefaults() {
        // Initialize vertex input state with empty configuration
        _vertexInputInfo.pNext                           = None;
        _vertexInputInfo.flags                           = 0;
        _vertexInputInfo.vertexBindingDescriptionCount   = 0;
        _vertexInputInfo.pVertexBindingDescriptions      = None;
        _vertexInputInfo.vertexAttributeDescriptionCount = 0;
        _vertexInputInfo.pVertexAttributeDescriptions    = None;

        // Setup default input assembly for triangle lists
        _inputAssembly.pNext                  = None;
        _inputAssembly.flags                  = 0;
        _inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        _inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Setup default viewport state (will be updated when viewport/scissor is set)
        _viewportState.pNext         = None;
        _viewportState.flags         = 0;
        _viewportState.viewportCount = 0;
        _viewportState.pViewports    = None;
        _viewportState.scissorCount  = 0;
        _viewportState.pScissors     = None;

        // Reset all our storage vectors to ensure clean state
        _shaderStages.clear();
        _vertexBindings.clear();
        _vertexAttributes.clear();
        _viewports.clear();
        _scissors.clear();
        _dynamicStates.clear();
    }

    bool VulkanPipelineBuilder::Validate() const {
        return false;
    }

    void VulkanPipelineBuilder::Reset() {}
}  // namespace x::vk