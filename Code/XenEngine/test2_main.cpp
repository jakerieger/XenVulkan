#include "Window.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "ShaderManager.hpp"

#include <memory>

int main() {
    using namespace x::vk;
    x::Window window(800, 600, "Title");

    auto win = window.GetWindow();
    auto context = std::make_unique<VulkanContext>(&win, true);

    while (!window.ShouldClose()) {
        window.PollEvents();
    }
}