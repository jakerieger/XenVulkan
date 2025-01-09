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
        ${ENGINE}/Vulkan/VulkanStruct.hpp
        ${ENGINE}/Vulkan/VulkanContext.hpp
        ${ENGINE}/Vulkan/VulkanContext.cpp
        ${ENGINE}/Vulkan/VulkanDevice.hpp
        ${ENGINE}/Vulkan/VulkanDevice.cpp
)

add_executable(test_main
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        test_main.cpp
)

target_link_libraries(Xen PRIVATE
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
        xFX
)

target_link_libraries(test_main PRIVATE
        Xen
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
        xFX
)