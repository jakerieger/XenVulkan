project(XenVulkan)

add_library(Xen SHARED
        ${COMMON}/Types.hpp
        ${COMMON}/Panic.inl
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        ${ENGINE}/XenEngine.hpp
        ${ENGINE}/XenEngine.cpp
        ${ENGINE}/Vulkan/VulkanStruct.hpp
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
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
        xFX
)