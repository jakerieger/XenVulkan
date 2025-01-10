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

    void VulkanPipelineBuilder::InitializeDefaults() {}

    bool VulkanPipelineBuilder::Validate() const {
        return false;
    }

    void VulkanPipelineBuilder::Reset() {}
}  // namespace x::vk