#version 450


layout (location = 0) in vec4 inColor;


layout (location = 0) out vec4 outColor;



layout(push_constant) uniform PushConstants {
    mat4 u_transform;
    int u_type;
    int u_letsize;
} pushConstants;

 vec4 thiscolor[3] ={
  inColor,
   {1.0f,0.1f,0.1f,0.5f},
   {0.8f,0.8f,0.1f,0.3f}
 };
void main() {
  
  outColor = thiscolor[pushConstants.u_type];

}
