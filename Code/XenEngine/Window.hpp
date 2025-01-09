// Author: Jake Rieger
// Created: 1/9/2025.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Types.hpp"
#include "Panic.inl"

namespace x {
    class Window {
    public:
        Window(u32 width, u32 height, cstr title);
        ~Window();

        GLFWwindow* GetWindow() const;
        bool ShouldClose() const;
        void PollEvents() const;

    private:
        GLFWwindow* _window = None;
    };
}  // namespace x
