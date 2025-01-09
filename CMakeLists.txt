cmake_minimum_required(VERSION 3.14)

set(CMAKE_TOOLCHAIN_FILE vcpkg/scripts/buildsystems/vcpkg.cmake)
project(XenVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(COMMON ${CMAKE_CURRENT_SOURCE_DIR}/Code/Common)
set(ENGINE ${CMAKE_CURRENT_SOURCE_DIR}/Code/XenEngine)
set(TOOLS ${CMAKE_CURRENT_SOURCE_DIR}/Code/Tools)
set(VENDOR ${CMAKE_CURRENT_SOURCE_DIR}/Code/Vendor)

include_directories(
        ${ENGINE}
        ${TOOLS}
        ${VENDOR}
        ${COMMON}
)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(${ENGINE})
add_subdirectory(${TOOLS}/xFX)