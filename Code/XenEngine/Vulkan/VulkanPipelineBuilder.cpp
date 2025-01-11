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
        _vertexBindings                                  = bindings;
        _vertexAttributes                                = attributes;
        _vertexInputInfo.vertexBindingDescriptionCount   = CAST<u32>(_vertexBindings.size());
        _vertexInputInfo.pVertexBindingDescriptions      = _vertexBindings.data();
        _vertexInputInfo.vertexAttributeDescriptionCount = CAST<u32>(_vertexAttributes.size());
        _vertexInputInfo.pVertexAttributeDescriptions    = _vertexAttributes.data();

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetInputAssembly(VkPrimitiveTopology topology,
                                                                   VkBool32 primitiveRestart) {
        _inputAssembly.topology               = topology;
        _inputAssembly.primitiveRestartEnable = primitiveRestart;

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetViewport(const VkViewport& viewport,
                                                              const VkRect2D& scissor) {
        _viewports                   = {viewport};
        _scissors                    = {scissor};
        _viewportState.viewportCount = 1;
        _viewportState.pViewports    = _viewports.data();
        _viewportState.scissorCount  = 1;
        _viewportState.pScissors     = _scissors.data();
        _dynamicViewportAndScissor   = false;
        std::erase_if(_dynamicStates, [](VkDynamicState state) {
            return state == VK_DYNAMIC_STATE_VIEWPORT || state == VK_DYNAMIC_STATE_SCISSOR;
        });

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetDynamicViewportAndScissor(u32 count) {
        _dynamicViewportAndScissor   = true;
        _viewportState.viewportCount = count;
        _viewportState.pViewports    = None;
        _viewportState.scissorCount  = count;
        _viewportState.pScissors     = None;

        auto addIfNotExists = [this](const VkDynamicState state) {
            if (std::ranges::find(_dynamicStates, state) == _dynamicStates.end()) {
                _dynamicStates.push_back(state);
            }
        };

        addIfNotExists(VK_DYNAMIC_STATE_VIEWPORT);
        addIfNotExists(VK_DYNAMIC_STATE_SCISSOR);

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetRasterizer(VkPolygonMode polygonMode,
                                                                VkCullModeFlags cullMode,
                                                                VkFrontFace frontFace,
                                                                f32 lineWidth) {
        _rasterizer.polygonMode = polygonMode;
        _rasterizer.cullMode    = cullMode;
        _rasterizer.frontFace   = frontFace;
        _rasterizer.lineWidth   = lineWidth;
        // - No depth clamping (fragments outside near/far planes are clipped)
        // - No rasterizer discard (fragments are processed normally)
        // - No depth bias (useful for shadow mapping, but not needed by default)

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetRasterizerDepthBias(const bool depthBias,
                                                                         const f32 depthBiasClamp) {
        _rasterizer.depthBiasEnable = CAST<VkBool32>(depthBias);
        _rasterizer.depthBiasClamp  = depthBiasClamp;

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetMultisampling(VkSampleCountFlagBits samples) {
        _multisampling.rasterizationSamples = samples;
        if (samples != VK_SAMPLE_COUNT_1_BIT) {
            _multisampling.sampleShadingEnable = VK_TRUE;
            _multisampling.minSampleShading    = 1.0f;  // full sample shading for best quality
        } else {
            _multisampling.sampleShadingEnable = VK_FALSE;
            _multisampling.minSampleShading    = 1.0f;
        }

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetDepthStencil(bool depthTest,
                                                                  bool depthWrite,
                                                                  VkCompareOp depthCompareOp) {
        _depthStencil.depthTestEnable       = depthTest;
        _depthStencil.depthWriteEnable      = depthWrite;
        _depthStencil.depthCompareOp        = depthCompareOp;
        _depthStencil.depthBoundsTestEnable = VK_FALSE;
        _depthStencil.minDepthBounds        = 0.0f;
        _depthStencil.maxDepthBounds        = 1.0f;
        _depthStencil.stencilTestEnable     = VK_FALSE;
        _depthStencil.front                 = {};
        _depthStencil.back                  = {};

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetColorBlending(
      bool enableBlending, const vector<VkPipelineColorBlendAttachmentState>& attachments) {
        _colorBlendAttachments         = attachments;
        _colorBlending.attachmentCount = CAST<u32>(_colorBlendAttachments.size());

        if (enableBlending) {
            for (auto& attachment : _colorBlendAttachments) {
                attachment.blendEnable         = VK_TRUE;
                attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attachment.colorBlendOp        = VK_BLEND_OP_ADD;
                attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                attachment.alphaBlendOp        = VK_BLEND_OP_ADD;
                attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
        }

        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetPipelineLayout(VkPipelineLayout layout) {
        _layout = layout;
        return *this;
    }

    VulkanPipelineBuilder& VulkanPipelineBuilder::SetRenderPass(VkRenderPass renderPass,
                                                                u32 subpass) {
        _renderPass = renderPass;
        _subpass    = subpass;
        return *this;
    }

    VulkanPipeline VulkanPipelineBuilder::Build(VkDevice device) const {
        if (!Validate()) Panic("Invalid pipeline configuration.");

        auto pipeline = VulkanPipeline(device);

        VulkanStruct<VkPipelineDynamicStateCreateInfo> dynamicState;
        if (!_dynamicStates.empty()) {
            dynamicState.dynamicStateCount = CAST<u32>(_dynamicStates.size());
            dynamicState.pDynamicStates    = _dynamicStates.data();
        }

        VulkanStruct<VkGraphicsPipelineCreateInfo> pipelineInfo;
        pipelineInfo.stageCount          = CAST<u32>(_shaderStages.size());
        pipelineInfo.pStages             = _shaderStages.data();
        pipelineInfo.pVertexInputState   = &_vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &_inputAssembly;
        pipelineInfo.pViewportState      = &_viewportState;
        pipelineInfo.pRasterizationState = &_rasterizer;
        pipelineInfo.pMultisampleState   = &_multisampling;
        pipelineInfo.pDepthStencilState  = &_depthStencil;
        pipelineInfo.pColorBlendState    = &_colorBlending;
        pipelineInfo.pDynamicState       = _dynamicStates.empty() ? None : &dynamicState;
        pipelineInfo.layout              = _layout;
        pipelineInfo.renderPass          = _renderPass;
        pipelineInfo.subpass             = _subpass;
        pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex   = -1;

        VkPipeline createdPipeline;
        if (vkCreateGraphicsPipelines(device,
                                      VK_NULL_HANDLE,
                                      1,
                                      &pipelineInfo,
                                      None,
                                      &createdPipeline) != VK_SUCCESS) {
            Panic("Failed to creatte graphics pipeline.");
        }

        pipeline._layout   = _layout;
        pipeline._pipeline = createdPipeline;

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

        _rasterizer.pNext                   = None;
        _rasterizer.flags                   = 0;
        _rasterizer.depthClampEnable        = VK_FALSE;
        _rasterizer.rasterizerDiscardEnable = VK_FALSE;
        _rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
        _rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
        _rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        _rasterizer.depthBiasEnable         = VK_FALSE;
        _rasterizer.depthBiasConstantFactor = 0.0f;
        _rasterizer.depthBiasClamp          = 0.0f;
        _rasterizer.depthBiasSlopeFactor    = 0.0f;
        _rasterizer.lineWidth               = 1.0f;

        _multisampling.pNext                 = None;
        _multisampling.flags                 = 0;
        _multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        _multisampling.sampleShadingEnable   = VK_FALSE;
        _multisampling.minSampleShading      = 1.0f;
        _multisampling.pSampleMask           = None;
        _multisampling.alphaToCoverageEnable = VK_FALSE;
        _multisampling.alphaToOneEnable      = VK_FALSE;

        _depthStencil.pNext                 = None;
        _depthStencil.depthTestEnable       = VK_FALSE;
        _depthStencil.depthWriteEnable      = VK_FALSE;
        _depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
        _depthStencil.depthBoundsTestEnable = VK_FALSE;
        _depthStencil.minDepthBounds        = 0.0f;
        _depthStencil.maxDepthBounds        = 1.0f;
        _depthStencil.stencilTestEnable     = VK_FALSE;
        _depthStencil.front                 = {};
        _depthStencil.back                  = {};

        _colorBlending.pNext             = None;
        _colorBlending.flags             = 0;
        _colorBlending.logicOpEnable     = VK_FALSE;          // Disable logical operations
        _colorBlending.logicOp           = VK_LOGIC_OP_COPY;  // Default to simple copy operation
        _colorBlending.attachmentCount   = 0;                 // Start with no color attachments
        _colorBlending.pAttachments      = None;
        _colorBlending.blendConstants[0] = 0.0f;  // Initialize blend constants to 0
        _colorBlending.blendConstants[1] = 0.0f;
        _colorBlending.blendConstants[2] = 0.0f;
        _colorBlending.blendConstants[3] = 0.0f;

        // Reset all our storage vectors to ensure clean state
        _shaderStages.clear();
        _vertexBindings.clear();
        _vertexAttributes.clear();
        _viewports.clear();
        _scissors.clear();
        _dynamicStates.clear();
    }

    bool VulkanPipelineBuilder::Validate() const {
        if (_layout == None) return false;
        if (_renderPass == None) return false;
        if (_shaderStages.empty()) return false;
        if (!_dynamicViewportAndScissor && (_viewports.empty() || _scissors.empty())) {
            return false;
        }
        if (_colorBlending.attachmentCount > 0 && _colorBlending.pAttachments == None) return false;
        return true;
    }

    void VulkanPipelineBuilder::Reset() {
        _layout                    = None;
        _renderPass                = None;
        _subpass                   = 0;
        _dynamicViewportAndScissor = false;
        _shaderStages.clear();
        _vertexBindings.clear();
        _vertexAttributes.clear();
        _viewports.clear();
        _scissors.clear();
        _dynamicStates.clear();
        _colorBlendAttachments.clear();
        InitializeDefaults();
    }
}  // namespace x::vk