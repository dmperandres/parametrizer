/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef BASIC_OBJECT3D_H
#define BASIC_OBJECT3D_H


#include <GL/glew.h>
#include <vector>
#include "vertex.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _basic_object3d
{
public:
  std::vector<_vertex3f> Vertices;
  std::vector<_vertex3f> Vertices_colors;
  std::vector<_vertex3f> Vertices_normals;
  std::vector<_vertex2f> Vertices_texture_coordinates;

  std::vector<_vertex3i> Triangles;
  std::vector<_vertex3f> Triangles_colors;
  std::vector<_vertex3f> Triangles_normals;

  std::vector<_vertex3f> Vertices_drawarray;
  std::vector<_vertex3f> Vertices_colors_drawarray;
  std::vector<_vertex3f> Vertices_triangles_colors_drawarray;
  std::vector<_vertex3f> Vertices_normals_drawarray;
  std::vector<_vertex3f> Vertices_triangles_normals_drawarray;
  std::vector<_vertex2f> Vertices_texture_coordinates_drawarray;

  _basic_object3d();
  
  void draw(){}
  int num_vertices_drawarray(){return int(Vertices_drawarray.size());}
  void update_drawing_data(int Initial_position1, int Num_vertices1);
  void compute_triangles_normals();
  void compute_vertices_normals();

protected:
  int Initial_position;
  int Num_vertices;
};
#endif
