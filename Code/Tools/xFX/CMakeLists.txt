project(XenVulkan)

add_library(xFX STATIC
        Fx.hpp
        Fx.cpp
)

find_package(unofficial-shaderc CONFIG REQUIRED)

target_link_libraries(xFX PRIVATE
        Vulkan::Vulkan
        unofficial::shaderc::shaderc
)