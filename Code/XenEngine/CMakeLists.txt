project(XenVulkan)

add_library(Xen STATIC
        ${COMMON}/Types.hpp
        ${COMMON}/Panic.inl
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        ${ENGINE}/XenEngine.hpp
        ${ENGINE}/XenEngine.cpp
        ${ENGINE}/Window.hpp
        ${ENGINE}/Window.cpp
        ${ENGINE}/ShaderManager.hpp
        ${ENGINE}/ShaderManager.cpp
        ${ENGINE}/Vulkan/VulkanStruct.hpp
        ${ENGINE}/Vulkan/VulkanContext.hpp
        ${ENGINE}/Vulkan/VulkanContext.cpp
        ${ENGINE}/Vulkan/VulkanDevice.hpp
        ${ENGINE}/Vulkan/VulkanDevice.cpp
        ${ENGINE}/Vulkan/VulkanPipeline.hpp
        ${ENGINE}/Vulkan/VulkanPipeline.cpp
        ${ENGINE}/Vulkan/VulkanPipelineBuilder.hpp
        ${ENGINE}/Vulkan/VulkanPipelineBuilder.cpp
        ${ENGINE}/Vulkan/VulkanSwapChain.hpp
        ${ENGINE}/Vulkan/VulkanSwapChain.cpp
)

add_executable(test2_main
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        test2_main.cpp
)

target_link_libraries(Xen PRIVATE
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
)

target_link_libraries(test2_main PRIVATE
        Xen
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
)