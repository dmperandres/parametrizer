#version 450 core

layout (location=15) uniform vec3 camera_position;

layout (location=20) uniform int mode_rendering;
layout (location=21) uniform int mode_color;
layout (location=22) uniform int illumination_active;
layout (location=23) uniform int mode_shading;
layout (location=24) uniform int texture_active;

layout (location=25) uniform vec3 color_point;
layout (location=26) uniform vec3 color_line;
layout (location=27) uniform vec3 color_fill;

layout (location=30) uniform vec4 light_position;
layout (location=31) uniform vec3 light_color;

layout (location=35) uniform vec3 ambient_coeff;

layout (location=40) uniform int selected_triangle;
layout (location=41) uniform int selected_triangle1;
layout (location=42) uniform int selected_triangle2;
layout (location=43) uniform int selected_triangle3;

layout (binding=0) uniform sampler2D texture_image;


float material_specular_exponent=1;

in vec3 color_vs;
in vec3 vertex_vs;
in vec3 normal_vs;
in vec2 texture_coordinates_vs;
in vec4 light_position_vs;
in vec3 triangle_color_vs;

out vec4 frag_color;


void main(void)
{
  vec3 color_aux;
  
  if (mode_rendering==0) frag_color=vec4(color_point,1); // point
  else{
    if (mode_rendering==1) frag_color=vec4(color_line,1); // line
    else{ // fill
      if (selected_triangle!=-1){
        if (selected_triangle==gl_PrimitiveID) frag_color=vec4(1,1,0,1);
        else{
          if (selected_triangle1==gl_PrimitiveID || selected_triangle2==gl_PrimitiveID || selected_triangle3==gl_PrimitiveID) frag_color=vec4(0,1,0,1);
          else frag_color=vec4(0.5,0.5,0.5,1);
        }
      }
      else{    
        if (illumination_active==0){
          if (texture_active==0){
            switch(mode_color){
            case 0:frag_color=vec4(color_fill,1);break;
            case 1:frag_color=vec4(triangle_color_vs,1);break;
            case 2:frag_color=vec4(color_vs,1);break;
            }
          }
          else{
            color_aux=vec3(texture(texture_image,texture_coordinates_vs));
            switch(mode_color){
            case 0:frag_color=vec4(0.5*color_aux+0.5*color_fill,1);break;
            case 1:frag_color=vec4(0.5*color_aux+0.5*triangle_color_vs,1);break;
            case 2:frag_color=vec4(0.5*color_aux+0.5*color_vs,1);break;
            }
          }
        }
        else{ // illumination active
          // diffuse
          vec3 light_direction;
          if (light_position_vs.w==0){
            light_direction=vec3(light_position_vs);
          }
          else{
            light_direction=vec3(light_position_vs)-vertex_vs;
          }
          light_direction=normalize(light_direction);
          vec3 normal_normalized=normalize(normal_vs);
          float diffuse_intensity = max(dot(light_direction,normal_normalized), 0.0);
          
          // specular
          float specular_intensity;
          if (diffuse_intensity>0){
            vec3 view_direction=camera_position-vertex_vs;
            view_direction=normalize(view_direction);
            vec3 reflected_direction = reflect(-light_direction,normal_normalized);
            specular_intensity = pow(max(dot(view_direction,reflected_direction), 0.0), material_specular_exponent);
          }
          else specular_intensity=0;         
                      
          // only illumination
          //vec4 color_layers=mix_colors(gl_PrimitiveID,color_fill);
          //vec4 color_layers=vec4(color_fill,1);
          
          if (texture_active==0){ // only illumination
            switch(mode_color){
            case 0:color_aux=color_fill;break;
            case 1:color_aux=triangle_color_vs;break;
            case 2:color_aux=color_vs;break;
            }
          }
          else{ // texture
            color_aux=vec3(texture(texture_image,texture_coordinates_vs));
            switch(mode_color){
            case 0:color_aux=0.5*color_aux+0.5*color_fill;break;
            case 1:color_aux=0.5*color_aux+0.5*triangle_color_vs;break;
            case 2:color_aux=0.5*color_aux+0.5*color_vs;break;
            }          
          }
          
          
          
          vec3 ambient_component=ambient_coeff*light_color*color_aux;
          vec3 diffuse_component = diffuse_intensity*light_color*color_aux;
  //         vec3 specular_component = specular_intensity*light_color*color_aux;
  //         frag_color=vec4(ambient_component+diffuse_component+specular_component,1);        
          frag_color=vec4(ambient_component+diffuse_component,1);        
        }
      }
    }
  }
}
