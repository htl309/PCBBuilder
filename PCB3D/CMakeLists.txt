if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # 要求VC正确设置C++识别宏
    add_compile_options(/Zc:__cplusplus)
    add_compile_options(/source-charset:utf-8)
    add_compile_options(/W4)
    add_compile_options(/Zi) # 生成调试信息，用于性能剖析

    # 指定用Unicode
    add_definitions("-DUNICODE" "-D_UNICODE" "-D_USE_MATH_DEFINES")
else()
    add_compile_options(-x c++ -std=c++20 -g -O2 -Wall -Wextra -Weffc++ -Wpedantic)
    set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    # set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()

include_directories(
${CMAKE_SOURCE_DIR}/PCB3D
)


add_library(pcb3d

      math/float.hpp
   math/vec2.hpp
   math/vec3.hpp
   pcb/base.hpp
   pcb/board.hpp
   pcb/layer.hpp
   pcb/layer_builder.hpp
   pcb/layer_lexer.hpp
   pcb/layer_parser.hpp
   pcb/line.hpp
   pcb/segment.hpp
   pcb/shape.hpp
   pcb/trail.hpp
   pcb/writer.hpp
   pcb/layer_builder.cpp
   pcb/layer_lexer.cpp
   pcb/layer_lexer_mem.cpp
   pcb/layer_parser.cpp
   pcb/writer.cpp
   pcb/verify.cpp

   pcb3d/builder3d.hpp
   pcb3d/pcbedge.hpp
   pcb3d/pcbface.hpp
   pcb3d/layer3d.hpp
   pcb3d/pcbshell.hpp
   pcb3d/trail3d.hpp
   pcb3d/stepwriter.hpp
   pcb3d/triangulation.hpp
   pcb3d/pcbexception.hpp

   pcb3d/builder3d.cpp
   pcb3d/pcbedge.cpp
   pcb3d/pcbface.cpp
   pcb3d/layer3d.cpp
   pcb3d/pcbshell.cpp
   pcb3d/trail3d.cpp
   pcb3d/stepwriter.cpp
   pcb3d/triangulation.cpp
)
 
