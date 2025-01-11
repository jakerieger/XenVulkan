#include "Filesystem.hpp"
#include "Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "Vulkan/VulkanPipelineBuilder.hpp"
#include "Vulkan/VulkanSwapChain.hpp"

#include <memory>

namespace helpers {
    struct VulkanPipelineObjects {
        VkAttachmentDescription colorAttachment         = {};
        VkAttachmentReference colorAttatchmentReference = {};
        VkSubpassDescription subpass                    = {};
        VkRenderPassCreateInfo renderPassInfo           = {};
        VkRenderPass renderPass;
        x::vk::VulkanStruct<VkPipelineLayoutCreateInfo> pipelineLayoutInfo;
        VkPipelineLayout pipelineLayout = {};
        VkViewport viewport             = {};
        VkRect2D scissor                = {};

        static VulkanPipelineObjects Create(VkDevice device, VkFormat imageFormat) {
            VulkanPipelineObjects objects;

            // Test pipeline builder
            objects.colorAttachment.format  = imageFormat;  // Match swapchain format
            objects.colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            objects.colorAttachment.loadOp =
              VK_ATTACHMENT_LOAD_OP_CLEAR;  // Clear color when frame starts
            objects.colorAttachment.storeOp =
              VK_ATTACHMENT_STORE_OP_STORE;  // Keep the color after rendering
            objects.colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            objects.colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            objects.colorAttatchmentReference.attachment = 0;
            objects.colorAttatchmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &objects.colorAttatchmentReference;

            objects.renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            objects.renderPassInfo.attachmentCount = 1;
            objects.renderPassInfo.pAttachments    = &objects.colorAttachment;
            objects.renderPassInfo.subpassCount    = 1;
            objects.renderPassInfo.pSubpasses      = &subpass;

            if (vkCreateRenderPass(device, &objects.renderPassInfo, nullptr, &objects.renderPass) !=
                VK_SUCCESS) {
                Panic("Failed to create render pass.");
            }

            if (vkCreatePipelineLayout(device,
                                       &objects.pipelineLayoutInfo,
                                       nullptr,
                                       &objects.pipelineLayout) != VK_SUCCESS) {
                Panic("Failed to create pipeline layout.");
            }

            objects.viewport.x        = 0.0f;
            objects.viewport.y        = 0.0f;
            objects.viewport.width    = CAST<float>(800);
            objects.viewport.height   = CAST<float>(600);
            objects.viewport.minDepth = 0.0f;
            objects.viewport.maxDepth = 1.0f;

            objects.scissor.offset = {0, 0};
            objects.scissor.extent = {800, 600};

            return objects;
        }
    };
}  // namespace helpers

int main() {
    using namespace x;
    using namespace x::vk;

    auto createShaderModule = [](VkDevice device, const vector<u8>& bytecode) {
        VulkanStruct<VkShaderModuleCreateInfo> createInfo;
        createInfo.codeSize = bytecode.size();
        createInfo.pCode    = RCAST<const u32*>(bytecode.data());
        VkShaderModule module;
        if (vkCreateShaderModule(device, &createInfo, None, &module) != VK_SUCCESS) {
            Panic("Failed to create shader module");
        }
        return module;
    };

    Window window(800, 600, "Title");

    // Test context and device creation
    auto win     = window.GetWindow();
    auto context = std::make_unique<VulkanContext>(&win, true);

    auto swapChain =
      std::make_unique<VulkanSwapChain>(context->GetDevice(), context->GetSurface(), 800, 600);

    auto builder = VulkanPipelineBuilder();
    vector<VkVertexInputBindingDescription> bindings;
    vector<VkVertexInputAttributeDescription> attributes;
    builder.SetVertexInput(bindings, attributes);

    auto vertBytes  = Filesystem::FileReader::ReadAllBytes("Shaders/Unlit.vert.spv");
    auto fragBytes  = Filesystem::FileReader::ReadAllBytes("Shaders/Unlit.frag.spv");
    auto vertModule = createShaderModule(context->GetDevice()->GetLogicalDevice(), vertBytes);
    auto fragModule = createShaderModule(context->GetDevice()->GetLogicalDevice(), fragBytes);

    builder.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertModule)
      .AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragModule);

    auto objects = helpers::VulkanPipelineObjects::Create(context->GetDevice()->GetLogicalDevice(),
                                                          swapChain->GetImageFormat());

    builder.SetViewport(objects.viewport, objects.scissor);
    builder.SetInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE)
      .SetRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
      .SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
      .SetDepthStencil(false, false)  // No depth testing for this simple test
      .SetColorBlending(false, {})    // No blending for this test
      .SetPipelineLayout(objects.pipelineLayout)
      .SetRenderPass(objects.renderPass);

    // auto pipeline = builder.Build(context->GetDevice());

    while (!window.ShouldClose()) {
        window.PollEvents();
    }

    vkDestroyPipelineLayout(context->GetDevice()->GetLogicalDevice(), objects.pipelineLayout, None);
    vkDestroyRenderPass(context->GetDevice()->GetLogicalDevice(), objects.renderPass, None);
    vkDestroyShaderModule(context->GetDevice()->GetLogicalDevice(), vertModule, None);
    vkDestroyShaderModule(context->GetDevice()->GetLogicalDevice(), fragModule, None);
}