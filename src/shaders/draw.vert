#version 450 core

layout (location=0) in vec3 vertex;
layout (location=1) in vec3 color;
layout (location=2) in vec3 triangle_normal;
layout (location=3) in vec3 vertex_normal;
layout (location=4) in vec2 texture_coordinates;
layout (location=5) in vec3 triangle_color;

layout (location=10) uniform mat4 model_matrix;
layout (location=11) uniform mat4 view_matrix;
layout (location=12) uniform mat4 projection_matrix;
layout (location=13) uniform mat3 normal_matrix;

layout (location=20) uniform int mode_rendering;
layout (location=21) uniform int mode_color;
layout (location=22) uniform int illumination_active;
layout (location=23) uniform int mode_shading;
layout (location=24) uniform int texture_active;

layout (location=25) uniform vec3 color_point;
layout (location=26) uniform vec3 color_line;
layout (location=27) uniform vec3 color_fill;

layout (location=30) uniform vec4 light_position;

out vec3 vertex_vs;
out vec3 color_vs;
out vec3 normal_vs;
out vec2 texture_coordinates_vs;
out vec4 light_position_vs;
out vec3 triangle_color_vs;

void main(void)
{
  vec3 normal_aux;
  if (mode_rendering==2){ // fill
    if (illumination_active==0){
      if (mode_color==2) color_vs=color; // color interpolation
    }
    else{ // illuminating
      if (mode_shading==1){ // flat
        normal_aux=triangle_normal; // flat
      }
      else{ // smooth
        normal_aux=vertex_normal; // phong  
      }
    }
  }
  vertex_vs=vec3(view_matrix*model_matrix*vec4(vertex,1));
  color_vs=color;
  normal_vs= vec3(view_matrix*model_matrix*vec4(normal_aux,0));
  light_position_vs=view_matrix*light_position;
  texture_coordinates_vs=texture_coordinates;
  triangle_color_vs=triangle_color;
  gl_Position=projection_matrix*vec4(vertex_vs,1);
}
