
glslc --target-env=vulkan1.2 Hybrid/CircleLet.task  -o Hybrid/CircleLet.task.spv
glslc --target-env=vulkan1.2 Hybrid/CircleLet.mesh  -o Hybrid/CircleLet.mesh.spv
glslc --target-env=vulkan1.2 Hybrid/CircleLet.frag  -o Hybrid/CircleLet.frag.spv

glslc --target-env=vulkan1.2 Hybrid/LineLet.task  -o Hybrid/LineLet.task.spv
glslc --target-env=vulkan1.2 Hybrid/LineLet.mesh  -o Hybrid/LineLet.mesh.spv
glslc --target-env=vulkan1.2 Hybrid/LineLet.frag  -o Hybrid/LineLet.frag.spv

glslc --target-env=vulkan1.2 Standard/Standard.vert  -o Standard/Standard.vert.spv
glslc --target-env=vulkan1.2 Standard/Standard.frag  -o Standard/Standard.frag.spv


pause