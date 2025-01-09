// Author: Jake Rieger
// Created: 1/9/2025.
//

#include "Window.hpp"

namespace x {
    Window::Window(const u32 width, const u32 height, cstr title) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(width, height, title, None, None);
        if (!_window) Panic("Failed to create window.");
    }

    Window::~Window() {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    GLFWwindow* Window::GetWindow() const {
        return _window;
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(_window);
    }

    void Window::PollEvents() const {
        if (_window) glfwPollEvents();
    }
}  // namespace x