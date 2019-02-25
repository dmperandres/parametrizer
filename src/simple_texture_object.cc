/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "simple_texture_object.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_simple_texture_object::_simple_texture_object()
{
  Vertices.resize(4);
  Vertices[0]=_vertex3f(0,0,-1);
  Vertices[1]=_vertex3f(1,0,-1);
  Vertices[2]=_vertex3f(1,1,-1);
  Vertices[3]=_vertex3f(0,1,-1);

  Triangles.resize(2);
  Triangles_normals.resize(2);
  Vertices_normals.resize(4);
  Vertices_texture_coordinates.resize(4);
  Vertices_colors.resize(Vertices.size());
  Triangles_colors.resize(Triangles.size());

  // create drawarrays
  // vertices
  Vertices_drawarray.resize(Vertices.size());
  for (unsigned int i=0;i<Vertices.size();i++){
    Vertices_drawarray[i]=Vertices[i];
  }
  // vertices colors
  Vertices_colors_drawarray.resize(Vertices.size());
  // vertices triangles colors
  Vertices_triangles_colors_drawarray.resize(Vertices.size());
  // vertices triangle normals
  Vertices_triangles_normals_drawarray.resize(Vertices.size());
  // vertices normals
  Vertices_normals_drawarray.resize(Vertices.size());
  // texture coordinates
  Vertices_texture_coordinates_drawarray.resize(Vertices.size());

  Initial_position=0;
  Num_vertices=Vertices_drawarray.size();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _simple_texture_object::draw()
{
  glDrawArrays(GL_LINE_LOOP,Initial_position,Num_vertices);
}
