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
    auto builder = VulkanPipelineBuilder();

    while (!window.ShouldClose()) {
        window.PollEvents();
    }
}