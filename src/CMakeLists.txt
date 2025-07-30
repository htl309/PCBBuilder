set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
${ROOT_DIR}
${CMAKE_SOURCE_DIR}/Graffiti
${CMAKE_SOURCE_DIR}/PCB3D
)

link_directories(
//${CMAKE_SOURCE_DIR}/build/Graffiti/Release
//${CMAKE_SOURCE_DIR}/build/PCB3D/Release
${CMAKE_SOURCE_DIR}/Graffiti
${CMAKE_SOURCE_DIR}/PCB3D
)

# 递归收集所有 .cpp 和 .h 文件
file(GLOB_RECURSE ALL_SRC_FILES
    "${ROOT_DIR}/*.cpp"
    "${ROOT_DIR}/*.c"
    "${ROOT_DIR}/*.h"
    "${ROOT_DIR}/*.hpp"
)


# 添加可执行文件
add_executable(PCBBuilder
 ${ALL_SRC_FILES}
)

set_target_properties(PCBBuilder PROPERTIES
    LINKER_LANGUAGE CXX  # 即使含有 .c 文件也指定为 C++
)

target_link_libraries(PCBBuilder 
Graffiti.lib
pcb3d.lib
glfw3.lib
glad.lib
Vulkan::Vulkan
)

set_target_properties(PCBBuilder PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if (MSVC)
    target_compile_options(PCBBuilder PRIVATE /utf-8)
endif()

# 设置 source_group 让 VS 显示目录结构
foreach(FILE_PATH IN LISTS ALL_SRC_FILES)
    file(RELATIVE_PATH REL_PATH "${ROOT_DIR}" "${FILE_PATH}")
    get_filename_component(DIR_PATH "${REL_PATH}" PATH)
    string(REPLACE "/" "\\" GROUP_PATH "${DIR_PATH}")
    source_group("${GROUP_PATH}" FILES "${FILE_PATH}")
endforeach()
