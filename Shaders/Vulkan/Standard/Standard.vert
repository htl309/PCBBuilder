#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 innormal;
layout(location = 2) in vec2 intexture;

layout(set = 0, binding = 0) uniform SceneData {
    mat4 u_ViewProjectionMatrix;
} scene;

layout(push_constant) uniform PushConstants {
    mat4 u_transform;
    int u_type;
} pushConstants;


void main() {
    gl_Position =scene.u_ViewProjectionMatrix*pushConstants.u_transform*vec4(inPosition, 1.0);
    // gl_Position =vec4(inPosition, 1.0);;
}
