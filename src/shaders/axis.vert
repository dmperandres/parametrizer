#version 450 core

layout (location=0) in vec3 vertex;
layout (location=1) in vec3 color;

layout (location=11) uniform mat4 view_matrix;
layout (location=12) uniform mat4 projection_matrix;

out vec3 color_vs;

void main(void)
{
  color_vs=color;
  gl_Position=projection_matrix*view_matrix*vec4(vertex,1);
}
