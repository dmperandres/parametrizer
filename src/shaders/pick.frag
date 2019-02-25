#version 450 core
out vec4 frag_color;

void main(void)
{
  vec3 Color;
  Color.r= (gl_PrimitiveID & 0x00FF0000) >> 16;
  Color.g= (gl_PrimitiveID & 0x0000FF00) >> 8;
  Color.b= gl_PrimitiveID & 0x000000FF;
  Color=Color/255.0;
  frag_color=vec4(Color,1);
}
