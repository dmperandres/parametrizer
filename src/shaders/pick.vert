#version 450 core
layout (location=0) in vec3 vertex;

layout (location=10) uniform mat4 model_matrix;
layout (location=11) uniform mat4 view_matrix;
layout (location=12) uniform mat4 projection_matrix;

void main(void)
{
  gl_Position=projection_matrix*view_matrix*model_matrix*vec4(vertex,1);
}
