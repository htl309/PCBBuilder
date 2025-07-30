#version 450 core


layout(std140,  binding = 0) uniform SceneData {
    mat4 u_ViewProjectionMatrix;
}scene;
uniform mat4 u_transform;
uniform int u_type;
 vec4 color[6] ={
   {0.0f,1.0f,0.0f,0.5f},
   {0.9f,0.0f,0.0f,0.3f},
   {0.0f,0.4f,0.9f,0.2f},
   {0.0f,0.45f,0.4f,0.08f},
   {0.8f,0.8f,0.1f,0.3f},
   {0.2f,0.2f,0.4f,0.3f},
   
 };


out vec4 FragColor;

void main()
{
    FragColor = color[u_type];
}