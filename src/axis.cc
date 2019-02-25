/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "axis.h"

using namespace _axis_ne;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_axis::_axis()
{
  // vertices
  Vertices.resize(6);
  Vertices[0]=_vertex3f(-MAX_AXIS_SIZE,0,0);
  Vertices[1]=_vertex3f(MAX_AXIS_SIZE,0,0);

  Vertices[2]=_vertex3f(0,-MAX_AXIS_SIZE,0);
  Vertices[3]=_vertex3f(0,MAX_AXIS_SIZE,0);

  Vertices[4]=_vertex3f(0,0,-MAX_AXIS_SIZE);
  Vertices[5]=_vertex3f(0,0,MAX_AXIS_SIZE);

  // vertices colors
  Vertices_colors.resize(6);
  Vertices_colors[0]=COLORS[1];
  Vertices_colors[1]=COLORS[1];

  Vertices_colors[2]=COLORS[2];
  Vertices_colors[3]=COLORS[2];

  Vertices_colors[4]=COLORS[3];
  Vertices_colors[5]=COLORS[3];

  // triangles normals
  Triangles_normals.resize(6);
  // vertices normals
  Vertices_normals.resize(6);
  // texture coordinates
  Vertices_texture_coordinates.resize(6);

  // create drawarrays
  // vertices and color
  Vertices_drawarray.resize(Vertices.size());
  Vertices_colors_drawarray.resize(Vertices_colors.size());

  for (unsigned int i=0;i<Vertices.size();i++){
    Vertices_drawarray[i]=Vertices[i];
    Vertices_colors_drawarray[i]=Vertices_colors[i];
  }

  Initial_position=0;
  Num_vertices=6;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _axis::draw()
{
  glDrawArrays(GL_LINES,Initial_position,Num_vertices);
}
