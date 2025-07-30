cmake_minimum_required(VERSION 3.18)

project(PCBBuilder
    VERSION 0.1.0
    DESCRIPTION "SDU ZhengQi"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIRS})

include_directories(
deps/
deps/GLFW/include
deps/spdlog/include
deps/glm/include
deps/glad/include
deps/tinygltf
)

link_directories(
deps/GLFW/lib
deps/glad/src
)

add_subdirectory(src)
add_subdirectory(Graffiti)
