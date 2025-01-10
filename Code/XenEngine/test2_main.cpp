#include "Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "ShaderManager.hpp"
#include "Vulkan/VulkanPipelineBuilder.hpp"

#include <memory>

int main() {
    using namespace x::vk;
    x::Window window(800, 600, "Title");

    // Test context and device creation
    auto win     = window.GetWindow();
    auto context = std::make_unique<VulkanContext>(&win, true);
    auto device  = context->GetDevice()->GetLogicalDevice();

    // Test pipeline builder
    auto builder  = VulkanPipelineBuilder();
    auto pipeline = builder.SetMultisampling(VK_SAMPLE_COUNT_4_BIT)
                      .SetDynamicViewportAndScissor(1)
                      .SetViewport({0, 0, 800, 600}, {0, 0, 800, 600})
                      .Build(device);

    while (!window.ShouldClose()) {
        window.PollEvents();
    }
}