#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 innormal;


uniform mat4 u_transform;


layout(std140,  binding = 0) uniform SceneData {
    mat4 u_ViewProjectionMatrix;
}scene;



void main()
{

    gl_Position =scene.u_ViewProjectionMatrix*u_transform*vec4(inPosition, 1.0);
}