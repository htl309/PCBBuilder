#version 450

layout (location = 0) in vec4 outColor;


layout (location = 0) out vec4 FragColor;



uniform   mat4 u_transform;
uniform  int u_type;
uniform int u_letsize;


 vec4 thiscolor[2] ={
  outColor,
   {0.0f,1.0f,0.0f,0.6f},
 };
void main() {
  
  FragColor = thiscolor[u_type];

}
