#version 450

layout(set = 0, binding = 0) uniform SceneData {
    mat4 u_ViewProjectionMatrix;
}scene;

layout(push_constant) uniform PushConstants {
    mat4 u_transform;
    int u_type;
} pushConstants;

layout(location = 0) out vec4 FragColor;
 vec4 color[6] ={
   {0.0f,1.0f,0.0f,0.5f},
   {0.9f,0.0f,0.0f,0.3f},
   {0.0f,0.4f,0.9f,0.2f},
   {0.0f,0.45f,0.4f,0.08f},
   {0.7f,0.7f,0.1f,0.3f},
   {0.2f,0.2f,0.4f,0.3f},
   
 };
void main() {
    FragColor = color[pushConstants.u_type];
}
