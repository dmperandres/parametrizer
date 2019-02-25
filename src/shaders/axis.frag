#version 450 core

in vec3 color_vs;

out vec4 frag_color;

void main(void)
{
  frag_color=vec4(color_vs,1);
}
