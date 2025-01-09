project(XenVulkan)

add_library(Xen SHARED
        ${COMMON}/Types.hpp
        ${COMMON}/Panic.inl
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        ${ENGINE}/XenEngine.hpp
        ${ENGINE}/XenEngine.cpp
)

add_executable(test_main
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        test_main.cpp
)

find_package(Vulkan REQUIRED)

target_link_libraries(Xen PRIVATE
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
)

target_link_libraries(test_main PRIVATE
        glm::glm-header-only
        glfw
        Vulkan::Vulkan
)