/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef PLY_H
#define PLY_H

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <assert.h>
#include <deque>
#include <list>
#include <random>

#include <fstream>
#include <sstream>
#include <iostream>
#include "vertex.h"
#include "parametrizador.h"

using namespace std;

class _parametrizer;


namespace _object3D_ns {


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  struct _vertex
  {
    _vertex3f Position;
    _vertex3f Normal;
    vector<int> Triangles;   // vector of indices to triangles that share the vertex
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  struct _edge
  {
    int Vertex1, Vertex2; // indices to vertex that conform the edge
    int Triangle1,Triangle2; // indices to triangles that conform the edge
    float Edge_size;  // Longitud de la arista
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  struct _triangle
  {
    _vertex3i Vertices;
    _vertex3i Edges;
    _vertex3f Angles;
    _vertex3f Normal;
    float Area;
    float Angle;
    // new for the parametrizer
    std::list<_parametrizer_ns::_triangle_order>::iterator Pointer;
    bool Used=false;
    bool Intersected=false;
    int Intersected_companion=-1;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/
  struct _vertex_edge
  {
    int Vertex;
    int Edge;
  };
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _object3D
{
public:
    vector<_object3D_ns::_vertex> Vertices;
    vector<_object3D_ns::_triangle> Triangles;
    vector<_object3D_ns::_edge> Edges;

    vector<_vertex2f> Projected_vertices;
    vector<_vertex3i> Projected_triangles;

    vector<_vertex3i> Triangles_shared_edges_order;
    float Greater_area;

    vector<_vertex3f> Vertices_colors;
    vector<_vertex3f> Triangles_colors;
    vector<_vertex2f> Vertices_texture_coordinates;

    vector<_vertex3f> Vertices_drawarray;
    vector<_vertex3f> Vertices_colors_drawarray;
    vector<_vertex3f> Vertices_triangles_colors_drawarray;
    vector<_vertex3f> Vertices_normals_drawarray;
    vector<_vertex3f> Vertices_triangles_normals_drawarray;
    vector<_vertex2f> Vertices_texture_coordinates_drawarray;

    _object3D();
    ~_object3D();

//    int open(const string &File_name1,string Mode);
//    void read_ply(vector<float> &Coordinates, vector<int> &Indices);
    void create(vector<float> &Coordinates,vector<int> &Indices);
//    void close();

    void create(vector<_vertex3f> &Vertices1, vector<_vertex3i> &Triangles1);

    void create_data_drawarrays();
    void update_texture_data_drawarrays(_parametrizer *Parametrizer1);

    void draw();

    float max_dimension();

    void compute_spherical_texture_coordinates();

    void compute_vertices_normals();

    // compute the projected triangles
    void compute_projected_triangles();

    // compute the order of the same edge in the shared triangle
    void compute_shared_edges_order();
    //
//    void write_ply();

    void get_triangles(int Selected_triangle, int &Selected_triangle1, int &Selected_triangle2, int &Selected_triangle3);

    void compute_vertices_angles();

    float min_value(){return Min_value;}
    float max_value(){return Max_value;}

protected:
  bool Bounding_box;
  _vertex3f Min_position;
  _vertex3f Max_position;
  _vertex3f Center_position;

  float Min_value;
  float Max_value;

private:
//  void get_token(std::istringstream &Line_stream1,std::string &Token1);
//  bool get_new_line(std::fstream &File,std::istringstream &Line_stream);

//  std::fstream File;

//  string File_name;
};
#endif
