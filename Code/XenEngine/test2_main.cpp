#include "Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "ShaderManager.hpp"
#include "Vulkan/VulkanPipelineBuilder.hpp"
#include "Vulkan/VulkanSwapChain.hpp"

#include <memory>

int main() {
    using namespace x;
    using namespace x::vk;
    Window window(800, 600, "Title");

    // Test context and device creation
    auto win     = window.GetWindow();
    auto context = std::make_unique<VulkanContext>(&win, true);

    auto swapChain =
      std::make_unique<VulkanSwapChain>(context->GetDevice(), context->GetSurface(), 800, 600);

    // Test pipeline builder
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format        = swapChain->GetImageFormat();  // Match swapchain format
    colorAttachment.samples       = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;   // Clear color when frame starts
    colorAttachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;  // Keep the color after rendering
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachRef = {};
    colorAttachRef.attachment            = 0;
    colorAttachRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount        = 1;
    renderPassInfo.pAttachments           = &colorAttachment;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(context->GetDevice()->GetLogicalDevice(),
                           &renderPassInfo,
                           None,
                           &renderPass) != VK_SUCCESS) {
        Panic("Failed to create render pass.");
    }

    VulkanStruct<VkPipelineLayoutCreateInfo> pipelineLayoutInfo;
    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(context->GetDevice()->GetLogicalDevice(),
                               &pipelineLayoutInfo,
                               None,
                               &pipelineLayout) != VK_SUCCESS) {
        Panic("Failed to create pipeline layout.");
    }

    auto builder = VulkanPipelineBuilder();
    vector<VkVertexInputBindingDescription> bindings;
    vector<VkVertexInputAttributeDescription> attributes;
    builder.SetVertexInput(bindings, attributes);

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = CAST<f32>(800);
    viewport.height     = CAST<f32>(600);
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor = {};
    scissor.offset   = {0, 0};
    scissor.extent   = {800, 600};

    builder.SetViewport(viewport, scissor);
    builder.SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE)
      .SetRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
      .SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
      .SetDepthStencil(false, false)  // No depth testing for this simple test
      .SetColorBlending(false, {})    // No blending for this test
      .SetPipelineLayout(pipelineLayout)
      .SetRenderPass(renderPass);

    auto pipeline = builder.Build(context->GetDevice());

    while (!window.ShouldClose()) {
        window.PollEvents();
    }

    vkDestroyPipelineLayout(context->GetDevice()->GetLogicalDevice(), pipelineLayout, None);
    vkDestroyRenderPass(context->GetDevice()->GetLogicalDevice(), renderPass, None);
}