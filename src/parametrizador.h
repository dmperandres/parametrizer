/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef PARAMETRIZER_H
#define PARAMETRIZER_H

#include <vector>
#include <deque>
#include <queue>
#include <chrono>
#include <list>
#include <memory>

#include <QString>
#include <QMatrix4x4>
#include <QVector4D>

#include "rectangle_packer.h"
#include "accuracy.h"

using namespace std;

class _object3D;

namespace _parametrizer_ns {
  const float DEFAULT_DIVISOR=2048;
  const float DEFAULT_DISTORTION=1;
  const float DEFAULT_PATCH_MAX=100.0;

  class _edge;
  class _free_point;

  const float MAX_FLOAT_VALUE=1e8;

  const float MIN_COLOR_VALUE=0.0f;
  const float MAX_COLOR_VALUE=0.9f;

  typedef enum {SEARCH_FORWARD,SEARCH_BACKWARD} _search_direction;

  typedef enum {ORDER_CRITERIA_AREA,ORDER_CRITERIA_ANGLE} _order_criteria;
  typedef enum {ORDER_DIRECTION_DESCENDING,ORDER_DIRECTION_ASCENDING} _order_direction;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _triangle_order
  {
  public:
    int Triangle=-1;
    float Value=0;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _point
  {
  public:
    int Position=-1;

    _vertex2f Patch_coordinates;
    _vertex2f Texture_coordinates;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _edge
  {
  public:
    int Position=-1; // debug

    int Triangle2=-1;
    int Pos_edge_triangle2=-1;
    list<_free_point>::iterator It_free_point;
    list<_point>::iterator It_point;
  };

  class _order_free_point;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _free_point
  {
  public:
    int Position=-1; // debug

    float Grade=-1;
    list<_edge>::iterator It_edge;
    list<_order_free_point>::iterator It_order_free_point;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _order_free_point
  {
  public:
    int Position=-1;

    float Grade=-1;
    list<_free_point>::iterator It_free_point;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _triangle
  {
  public:
    int Model_triangle=-1;
    list<_point>::iterator It_points[3];
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _triangle_deformation
  {
  public:
    int Object3D_triangle;
    _vertex2f Vertices[3];
    float Deformation=0;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _order_patch
  {
  public:
    float Area=-1;
    int Patch_index=-1;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _common_triangle
  {
  public:
    int Model_triangle=-1;
    list<_edge>::iterator It_edge;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _patch
  {
  public:

    list<_point> List_points;
    list<_edge> List_edges; // patch_front
    vector<_triangle> Vector_triangles;
    list<_free_point> List_free_points;

    list<_order_free_point> List_order_free_points;

    list<_common_triangle> List_common_triangles;
    list<_triangle_deformation> List_triangles_deformation;

    _vertex3f Color;
    float Area=0;

    _vertex2f Dimension;
    _vertex2f Min_position=_vertex2f(MAX_FLOAT_VALUE,MAX_FLOAT_VALUE);
    _vertex2f Max_position=_vertex2f(-MAX_FLOAT_VALUE,-MAX_FLOAT_VALUE);

    _patch(_object3D *Object3D1,list<_parametrizer_ns::_triangle_order> *List_triangles_order1,float Distortion1, unsigned int Max_triangles1);
    ~_patch();
    void add_initial_triangle(int Triangle1);
    void expand();

    void search_common_triangles_direction(list<_edge>::iterator It_edge, int Pos_triangle2, int Pos_edge_triangle2, _search_direction Direction, bool &Ring);
    void search_common_triangles(_free_point Free_point1);
    _vertex2f compute_projected_point(list<_point>::iterator It_point1, list<_point>::iterator It_point2, int Pos_triangle2, int Pos_edge_triangle2);

    list<_point>::iterator insert_new_point(_vertex2f New_vertex);

    void insert_new_triangles(list<_point>::iterator It_point);

    void insert_new_triangle(int Pos_triangle2, int Pos_edge_triangle2, list<_point>::iterator It1, list<_point>::iterator It2, list<_point>::iterator It3);

    void triangles_used();
    void triangle_used(int Pos_triangle);

    void insert_edge(int Pos_triangle1, int Pos_edge_triangle1, list<_edge>::iterator It_edge, list<_point>::iterator It_point);

    void insert_front_edges_multiple(list<_point>::iterator It_new_point);

    void insert_front_edges(int Pos_triangle2, int Pos_edge_triangle2, list<_edge>::iterator It_edge, list<_point>::iterator It_point1, list<_point>::iterator It_point2);

    void add_new_free_points(list<_edge>::iterator It_new_edge, int Num_points);
    _vertex2f compute_mean_point();
    void create_deformed_triangles(_vertex2f New_point);
    float compute_triangle_deformation(_vertex2f Point0, _vertex2f Point1, _vertex2f Point2, int Pos_triangle2);
    float compute_deformation(float &Mean_error1, float &Max_error1, unsigned int &Num_triangles_with_error1);

    list<_edge>::iterator remove_edges();
    void remove_free_point(list<_edge>::iterator It_edge);

    bool add_single_point(_free_point Free_point1);

    void add_free_point(_free_point Free_point1);

    void compute_boundingbox(_vertex2f Point);
    bool check_intersection_with_front(_vertex2f Vertex1,_vertex2f Vertex2,_vertex2f Vertex3);
    int compute_two_lines_intersection(_vertex2f Vertex1,_vertex2f Vertex2,_vertex2f Vertex3,_vertex2f Vertex4);

    bool check_point_is_outside_front(_vertex2f New_vertex2d);

    void compute_area_dimensions();

    void update_errors(float Mean_error1,float Max_error1,unsigned int Num_triangles_with_error1);

    float mean_error(){return Mean_error;}
    float max_error(){return Max_error;}
    unsigned int num_triangles_with_error(){return Num_triangles_with_error;}

  protected:
    _object3D *Object3D;
    list<_parametrizer_ns::_triangle_order> *List_triangles_order;
    float Distortion=0;
    float Patch_max=100;

    int Edges_counter=0;
    int Points_counter=0;
    int Free_points_counter=0;
    int Order_free_points_counter=0;
    bool Same_triangle;

    unsigned int Max_triangles;

    float Mean_error; // mean error of triangles that have error
    float Max_error; // max error of a triangle
    unsigned int Num_triangles_with_error;
  };
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _parametrizer
{
public:
  _object3D *Object3D;

  // list for getting the seed triangles orderd by area size
  list<_parametrizer_ns::_triangle_order> List_triangles_order;
  vector<unique_ptr<_parametrizer_ns::_patch>> Vector_patches;

  _parametrizer();
  ~_parametrizer();

  void clear_data();

  void set_ply_model(_object3D *Object3D1);
  void set_distorsion(float Distortion1);
  void set_order_criterion(int Value);
  void set_patch_max(float Value1);

  void run_parametrization();

  int get_triangle_from_list();

  float distortion(){return Distortion;}

  void adjust_patches();

  float patch_max(){return Patch_max;}

  float time_parametrization(){return float(Time_parametrization);}
  float time_packing(){return float(Time_packing);}

  float total_error(){return Total_error;}
  float mean_error(){return Mean_error;}
  float max_error(){return Max_error;}

private:
  double Time_parametrization;
  double Time_packing;
  float Distortion;   // Grado de distorsión permitido.
  _parametrizer_ns::_order_criteria Order_criteria;
  _parametrizer_ns::_order_direction Order_direction;

  float Patch_max;
  unsigned int Max_triangles;

  float Total_error; // mean error of all triangles
  float Mean_error; // mean error of triangles that have error
  float Max_error; // max error of a triangle
  unsigned int Num_triangles_with_error;
};
#endif
