// Author: Jake Rieger
// Created: 1/9/25.
//

#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "Types.hpp"
#include "Panic.inl"
#include "VulkanPipeline.hpp"
#include "VulkanStruct.hpp"

namespace x::vk {
    class VulkanShader {
    public:
        VulkanShader() = default;
    };

    class VulkanPipelineBuilder {
    public:
        VulkanPipelineBuilder();
        ~VulkanPipelineBuilder() = default;

        // Prevent copying to avoid confusion about pipeline state ownership
        VulkanPipelineBuilder(const VulkanPipelineBuilder&)            = delete;
        VulkanPipelineBuilder& operator=(const VulkanPipelineBuilder&) = delete;

        // Allow moving to support builder pattern usage in STL containers
        VulkanPipelineBuilder(VulkanPipelineBuilder&&) noexcept            = default;
        VulkanPipelineBuilder& operator=(VulkanPipelineBuilder&&) noexcept = default;

        // Shader stage configuration
        // TODO: Rework the shader system and integrate that here, for now this won't do anything
        VulkanPipelineBuilder& AddShaderStage(VkShaderStageFlagBits stage,
                                              const VulkanShader& shader);

        // Vertex input configuration
        VulkanPipelineBuilder&
        SetVertexInput(const vector<VkVertexInputBindingDescription>& bindings,
                       const vector<VkVertexInputAttributeDescription>& attributes);

        // Input assembly configuration
        VulkanPipelineBuilder&
        SetInputAssembly(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                         VkBool32 primitiveRestart    = VK_FALSE);

        // Viewport and scissor configuration
        VulkanPipelineBuilder& SetViewport(const VkViewport& viewport, const VkRect2D& scissor);
        VulkanPipelineBuilder& SetDynamicViewportAndScissor(u32 count = 1);

        // Rasterization configuration
        VulkanPipelineBuilder& SetRasterizer(VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
                                             VkCullModeFlags cullMode  = VK_CULL_MODE_BACK_BIT,
                                             VkFrontFace frontFace     = VK_FRONT_FACE_CLOCKWISE,
                                             f32 lineWidth             = 1.0f);

        VulkanPipelineBuilder& SetRasterizerDepthBias(bool depthBias, f32 depthBiasClamp);

        // Multisampling configuration
        VulkanPipelineBuilder&
        SetMultisampling(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

        // Depth and stencil configuration
        VulkanPipelineBuilder& SetDepthStencil(bool depthTest             = true,
                                               bool depthWrite            = true,
                                               VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS);

        // Color blending configuration
        VulkanPipelineBuilder&
        SetColorBlending(bool enableBlending                                            = false,
                         const vector<VkPipelineColorBlendAttachmentState>& attachments = {});

        // Pipeline layout configuration
        VulkanPipelineBuilder& SetPipelineLayout(VkPipelineLayout layout);

        // Render pass configuration
        VulkanPipelineBuilder& SetRenderPass(VkRenderPass renderPass, u32 subpass = 0);

        VulkanPipeline Build(VkDevice device) const;

    private:
        void InitializeDefaults();
        bool Validate() const;
        void Reset();

        // Pipeline state storage
        vector<VkPipelineShaderStageCreateInfo> _shaderStages;
        // VulkanStruct is wrapper for automatically assigning the `sType` property with the correct
        // enum
        VulkanStruct<VkPipelineVertexInputStateCreateInfo> _vertexInputInfo;
        VulkanStruct<VkPipelineInputAssemblyStateCreateInfo> _inputAssembly;
        VulkanStruct<VkPipelineViewportStateCreateInfo> _viewportState;
        VulkanStruct<VkPipelineRasterizationStateCreateInfo> _rasterizer;
        VulkanStruct<VkPipelineMultisampleStateCreateInfo> _multisampling;
        VulkanStruct<VkPipelineDepthStencilStateCreateInfo> _depthStencil;
        VulkanStruct<VkPipelineColorBlendStateCreateInfo> _colorBlending;

        // Supporting state storage
        vector<VkVertexInputBindingDescription> _vertexBindings;
        vector<VkVertexInputAttributeDescription> _vertexAttributes;
        vector<VkPipelineColorBlendAttachmentState> _colorBlendAttachments;
        vector<VkDynamicState> _dynamicStates;

        // Pipeline configuration
        VkPipelineLayout _layout;
        VkRenderPass _renderPass;
        u32 _subpass;

        // Viewport and scissor state
        vector<VkViewport> _viewports;
        vector<VkRect2D> _scissors;
        bool _dynamicViewportAndScissor;
    };
}  // namespace x::vk
