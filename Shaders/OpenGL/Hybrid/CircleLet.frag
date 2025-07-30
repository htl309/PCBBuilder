#version 450

in vec4 outColor;
out vec4 FragColor;

uniform   mat4 u_transform;
uniform  int u_type;
uniform int u_letsize;

vec4 thiscolor[3] ={
  outColor,
   {1.0f,0.0f,0.0f,0.5f},
   {0.8f,0.8f,0.1f,0.3f}
};

void main() {
  FragColor = thiscolor[u_type];
}