/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include <math.h>
#include <string>
#include "object3D.h"
#include "parametrizador.h"

//#define DEBUG

using namespace std;
using namespace _parametrizer_ns;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

list<_edge>::iterator next(list<_edge> &List,list<_edge>::iterator It,int Num=1)
{
  list<_edge>::iterator Next=It;
  int Count=0;

  do{
    Next=next(It);
    if (Next==List.end()) Next=next(Next);
    Count++;
  } while(Count<Num);

  return(Next);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

list<_edge>::iterator prev(list<_edge> &List,list<_edge>::iterator It,int Num=1)
{
  list<_edge>::iterator Previous=It;
  int Count=0;

  do{
    Previous=prev(Previous);
    if (Previous==List.end()) Previous=prev(Previous);
    Count++;
  } while(Count<Num);
  return(Previous);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool compare_triangle_value_descending(const _triangle_order& First, const _triangle_order& Second)
{
  // order: first greater values
  if (First.Value>Second.Value) return true;
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool compare_triangle_value_ascending(const _triangle_order& First, const _triangle_order& Second)
{
  // order: first smaller values
  if (First.Value<Second.Value) return true;
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool compare_grade(const _order_free_point& First, const _order_free_point& Second)
{
  // order: first greater values
  if (First.Grade>Second.Grade) return true;
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool compare_patches(const _order_patch& First, const _order_patch& Second)
{
  // order: first greater values
  if (First.Area>Second.Area) return true;
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_parametrizer::_parametrizer()
{
  list<_edge> List;
  Distortion=DEFAULT_DISTORTION;
  Order_criteria=_parametrizer_ns::ORDER_CRITERIA_AREA;
  Order_direction=_parametrizer_ns::ORDER_DIRECTION_DESCENDING;

  Patch_max=DEFAULT_PATCH_MAX;
  Max_triangles=0;

  Object3D=nullptr;

  Total_error=0; // mean error of all triangles
  Mean_error=0; // mean error of triangles that have error
  Max_error=0; // max error of a triangle
  Num_triangles_with_error=0;

  Time_parametrization=0;
  Time_packing=0;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_parametrizer::~_parametrizer()
{
  List_triangles_order.clear();
  Vector_patches.clear();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _parametrizer::clear_data()
{
  Vector_patches.clear();
  List_triangles_order.clear();

  // create a list of free triangles. Each triangle has the index to the triangle in the ply model and the corresponding area
  _triangle_order Triangle_order;

  List_triangles_order.clear();
  for (uint i=0;i<Object3D->Triangles.size();i++){
    Triangle_order.Triangle=int(i);

    switch(Order_criteria){
    case ORDER_CRITERIA_AREA:Triangle_order.Value=Object3D->Triangles[i].Area;break;
    case ORDER_CRITERIA_ANGLE:Triangle_order.Value=Object3D->Triangles[i].Angle;break;
    }

    List_triangles_order.push_back(Triangle_order);
    Object3D->Triangles[i].Pointer=--List_triangles_order.end();
    Object3D->Triangles[i].Used=false;
  }

  // order by area
  switch (Order_direction) {
  case ORDER_DIRECTION_DESCENDING:List_triangles_order.sort(compare_triangle_value_descending);break;
  case ORDER_DIRECTION_ASCENDING:List_triangles_order.sort(compare_triangle_value_ascending);break;
  }

  Total_error=0; // mean error of all triangles
  Mean_error=0; // mean error of triangles that have error
  Max_error=0; // max error of a triangle
  Num_triangles_with_error=0;

  Time_parametrization=0;
  Time_packing=0;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _parametrizer::set_ply_model(_object3D *Object3D1)
{
  Object3D = Object3D1;
  Max_triangles=static_cast<unsigned int>(float(Object3D->Triangles.size())*Patch_max/100.0f);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _parametrizer::set_distorsion(float Distortion1)
{
  Distortion = Distortion1;

  if (Distortion < 0) Distortion = 0.0;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _parametrizer::set_patch_max(float Value1)
{
  Patch_max=Value1;
  Max_triangles=static_cast<unsigned int>(float(Object3D->Triangles.size())*Patch_max/100.0f);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _parametrizer::set_order_criterion(int Value)
{
  switch (Value){
  case 0:// Area descending
    Order_criteria=ORDER_CRITERIA_AREA;
    Order_direction=ORDER_DIRECTION_DESCENDING;
    break;
  case 1:// Area ascending
    Order_criteria=ORDER_CRITERIA_AREA;
    Order_direction=ORDER_DIRECTION_ASCENDING;
    break;
  case 2:// Angle descending
    Order_criteria=ORDER_CRITERIA_ANGLE;
    Order_direction=ORDER_DIRECTION_DESCENDING;
    break;
  case 3:// Angle ascending
    Order_criteria=ORDER_CRITERIA_ANGLE;
    Order_direction=ORDER_DIRECTION_ASCENDING;
    break;
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _parametrizer::run_parametrization()
{
  // clear
  clear_data();

  auto start = std::chrono::high_resolution_clock::now();

  while(List_triangles_order.size()>0){
    // create the patch. It needs the ply_object to get some information
    unique_ptr<_patch> Patch=make_unique<_patch>(Object3D,&List_triangles_order,Distortion/100.0f,Max_triangles);
    // add the initial triangle (the one with the biggest area)
    Patch->add_initial_triangle(get_triangle_from_list());
    // expand the border
    Patch->expand();
    // save error data
    Mean_error+=Patch->mean_error();
    Num_triangles_with_error+=Patch->num_triangles_with_error();
    if (Patch->max_error()>Max_error) Max_error=Patch->max_error();
    // the patch cannot be extended, it is saved
    Vector_patches.push_back(move(Patch));
  }

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  Time_parametrization=elapsed.count();

  Total_error=Mean_error/Object3D->Triangles.size();
  Mean_error/=Num_triangles_with_error;

  Total_error=_accuracy_ns::accuracy(Total_error)*100.0f;
  Mean_error=_accuracy_ns::accuracy(Mean_error)*100.0f;
  Max_error*=100.0f;

//  cout << "Parametrizacion=" << float(Time_parametrization) << " segundos"  << endl;
//  cout << "Total error=" << Total_error << " Mean error=" << Mean_error << " Max error=" << Max_error << endl;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

int _parametrizer::get_triangle_from_list()
{
  int Triangle=(*List_triangles_order.begin()).Triangle;

  // the triangle is going to be used
  Object3D->Triangles[static_cast<unsigned long>(Triangle)].Pointer=list<_triangle_order>::iterator(nullptr);
  Object3D->Triangles[static_cast<unsigned long>(Triangle)].Used=true;
  // remove the triangle from the list
  List_triangles_order.pop_front();
  return Triangle;
}

// This function does the packing and normalizes the coordinates

void _parametrizer::adjust_patches()
{
  _vertex2f Min_position;
  _vertex2f Max_position;
  _vertex2f Size;
  _vertex2f Patch_coordinates;
  _vertex2f Texture_coordinates;
//  float Side;

  auto start = std::chrono::high_resolution_clock::now();

  // For each patch, the rectangle is created
  vector<_rectangle_packer_ns::_rectangle> Rectangles;

  Rectangles.resize(Vector_patches.size());

  // set the data of each rectangle from each patch
  for (unsigned int Patch=0;Patch<Vector_patches.size();Patch++){
    Rectangles[Patch].Area=_accuracy_ns::accuracy(Vector_patches[Patch]->Area);
    Rectangles[Patch].Size.x=_accuracy_ns::accuracy(Vector_patches[Patch]->Dimension.x);
    Rectangles[Patch].Size.y=_accuracy_ns::accuracy(Vector_patches[Patch]->Dimension.y);
    Rectangles[Patch].Position=_vertex2f(-1,-1);
    Rectangles[Patch].Rotate=false;
  }

  _rectangle_packer_ns::_rectangles_packer Rectangle_packer;
  float Big_side;

  // Do the packing. Get the max size
  Big_side=Rectangle_packer.pack(Rectangles);

  float Border=Rectangle_packer.border();

  Border/=Big_side;

  float Factor_x,Factor_y;
  list<_point>::iterator It_p;
  list<_point>::iterator It_p1;
  float Origin_x;
  float Origin_y;
  float Corner_x;
  float Corner_y;

  for (unsigned int i=0;i<Rectangles.size();i++){
    // compute the corners of the rectangle
    if (Rectangles[i].Rotate==false){
      Origin_x=Rectangles[i].Position.x;
      Origin_y=Rectangles[i].Position.y;

      Corner_x=Origin_x+Rectangles[i].Size.x;
      Corner_y=Origin_y+Rectangles[i].Size.y;

      // now normalize
      Origin_x/=Big_side;
      Origin_y/=Big_side;
      Corner_x/=Big_side;
      Corner_y/=Big_side;

      Origin_x+=Border;
      Origin_y+=Border;
      Corner_x-=Border;
      Corner_y-=Border;

      // now a linear interpolation has to be applied to pass from local coordinates of each patch to normalized coordinates
      Min_position=Vector_patches[i]->Min_position;
      Max_position=Vector_patches[i]->Max_position;
    }
    else{ // the rectangle is rotated -> interchange the x and y coordinates
      Origin_x=Rectangles[i].Position.x;
      Origin_y=Rectangles[i].Position.y;

      Corner_x=Origin_x+Rectangles[i].Size.y;
      Corner_y=Origin_y+Rectangles[i].Size.x;

      // now normalize
      Origin_x/=Big_side;
      Origin_y/=Big_side;
      Corner_x/=Big_side;
      Corner_y/=Big_side;

      Origin_x+=Border;
      Origin_y+=Border;
      Corner_x-=Border;
      Corner_y-=Border;

      if (Corner_x<Origin_x || Corner_y<Origin_y) cout << "menor" << endl;

      Min_position.x=Vector_patches[i]->Min_position.y;
      Min_position.y=Vector_patches[i]->Max_position.x;

      Max_position.x=Vector_patches[i]->Max_position.y;
      Max_position.y=Vector_patches[i]->Min_position.x;
    }

    Factor_x=(Corner_x-Origin_x)/(Max_position.x-Min_position.x);
    Factor_y=(Corner_y-Origin_y)/(Max_position.y-Min_position.y);

    if (Rectangles[i].Rotate==false){
      for (auto It=Vector_patches[i]->List_points.begin();It!=Vector_patches[i]->List_points.end();It++){
        Patch_coordinates=(*It).Patch_coordinates;

        Texture_coordinates.x=Factor_x*(Patch_coordinates.x-Min_position.x)+Origin_x;
        Texture_coordinates.y=Factor_y*(Patch_coordinates.y-Min_position.y)+Origin_y;
        (*It).Texture_coordinates=Texture_coordinates;
      }
    }
    else{
      for (auto It=Vector_patches[i]->List_points.begin();It!=Vector_patches[i]->List_points.end();It++){
        Patch_coordinates=(*It).Patch_coordinates;

        Texture_coordinates.x=Factor_x*(Patch_coordinates.y-Min_position.x)+Origin_x;
        Texture_coordinates.y=Factor_y*(Patch_coordinates.x-Min_position.y)+Origin_y;
        (*It).Texture_coordinates=Texture_coordinates;
      }
    }
  }

  auto finish = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = finish - start;
  Time_packing=elapsed.count();

//  cout << "Packing=" << float(Time_packing) << " segundos"  << endl;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_patch::_patch(_object3D *Object3D1, list<_triangle_order> *List_triangles_order1, float Distortion1, unsigned int Max_triangles1)
{
  Distortion=Distortion1;
  Max_triangles=Max_triangles1;

  Object3D=Object3D1;
  List_triangles_order=List_triangles_order1;

  random_device RD;
  mt19937 MT(RD());
  uniform_real_distribution<float> Random(MIN_COLOR_VALUE,MAX_COLOR_VALUE);

  Color=_vertex3f(Random(MT),Random(MT),Random(MT));

//  Total_error=0; // mean error of all triangles
  Mean_error=0; // mean error of triangles that have error
  Max_error=0; // max error of a triangle
  Num_triangles_with_error=0;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_patch::~_patch()
{
  List_points.clear();
  List_edges.clear(); // patch_front
  Vector_triangles.clear();
  List_free_points.clear();

  List_order_free_points.clear();
  List_common_triangles.clear();
  List_triangles_deformation.clear();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::add_initial_triangle(int Triangle1)
{
  _point Point_1,Point_2;
  _triangle Triangle;

  // create the edges of the front, the points and the data of the new triangle
  for (int i=0;i<3;i++){
    // Create one point from each vertex of the triangle
    Point_1.Position=Points_counter++;
    Point_1.Patch_coordinates=Object3D->Projected_vertices[static_cast<unsigned long>(Object3D->Projected_triangles[static_cast<unsigned long>(Triangle1)][i])];

    compute_boundingbox(Point_1.Patch_coordinates);

    List_points.push_back(Point_1);

    // pointer to the new point
    list<_point>::iterator It_point=prev(List_points.end());
    Triangle.It_points[i]=It_point;

    insert_edge(Triangle1,i,List_edges.end(),It_point);
  }

  Triangle.Model_triangle=Triangle1;
  // save the triangle
  Vector_triangles.push_back(Triangle);
  // add the new free points
  add_new_free_points(List_edges.begin(),3);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::expand()
{
  _order_free_point Order_free_point;
  _free_point Free_point;
  list<_free_point>::iterator It_lfp;
  list<_free_point>::iterator It_lfp1;
  list<_edge>::iterator It_edge;
  list<_edge>::iterator It_edge1;
  _point Point;
  float THRESHOLD=0;
  unsigned int Counter=0;

  // get the point with bigger grade -> the length added is the lower one
  while (List_order_free_points.size()>0 and Counter<Max_triangles){
    // get the point
    Order_free_point=*List_order_free_points.begin();
    It_lfp=Order_free_point.It_free_point;
    Free_point=*It_lfp;
    if (Free_point.Grade>THRESHOLD){
      add_free_point(Free_point);
      Counter++;
    }
    else break; // the point doesn't complies with the condition
  }

  // compute dimensions and size
  compute_area_dimensions();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::search_common_triangles_direction(list<_edge>::iterator It_edge,int Pos_triangle2,int Pos_edge_triangle2,_search_direction Direction,bool &Ring)
{
  bool Comparted_triangle=true;
  list<_edge>::iterator It_previous_edge;
  list<_edge>::iterator It_next_edge;
  int Pos_previous_triangle2;
  int Pos_next_triangle2;
  int Pos_previous_edge_triangle2,Pos_next_edge_triangle2;

  It_next_edge=It_edge;
  Pos_next_triangle2=Pos_triangle2;
  Pos_next_edge_triangle2=Pos_edge_triangle2;

  do{
    It_previous_edge=It_next_edge;

    if (Direction==SEARCH_FORWARD){
      It_next_edge=next(List_edges,It_previous_edge);
    }
    else{
      It_next_edge=prev(List_edges,It_previous_edge);
    }

    if (It_next_edge!=It_edge){
      Pos_previous_triangle2=Pos_next_triangle2;
      Pos_next_triangle2=(*It_next_edge).Triangle2;

      if (Pos_previous_triangle2!=Pos_next_triangle2 && Pos_next_triangle2!=-1){
        // check that the triangle to include has not been used previously
        if (Object3D->Triangles[static_cast<unsigned long>(Pos_next_triangle2)].Used==true) Comparted_triangle=false;
        else{ // Is it neighbor?
          Pos_previous_edge_triangle2=Pos_next_edge_triangle2;
          Pos_next_edge_triangle2=(*It_next_edge).Pos_edge_triangle2;

          // check the points of both triangles
          if (Object3D->Triangles[static_cast<unsigned long>(Pos_previous_triangle2)].Vertices[(Pos_previous_edge_triangle2+2)%3]==Object3D->Triangles[static_cast<unsigned long>(Pos_next_triangle2)].Vertices[(Pos_next_edge_triangle2+2)%3]){
            // the triangles are neighbors and has a common vertex
            // save data
            _common_triangle Common_triangle;

            Common_triangle.It_edge=It_next_edge;

            if (Direction==SEARCH_FORWARD){
              List_common_triangles.push_back(Common_triangle);
            }
            else{
              List_common_triangles.push_front(Common_triangle);
            }
          }
          else Comparted_triangle=false;
        }
      }
      else{
        if (Pos_next_triangle2!=-1 && Pos_previous_triangle2!=-1){

//          cout << "Same triangle" << endl;
          // the same triangle is comparted
          Same_triangle=true;
          _common_triangle Common_triangle;

          Common_triangle.It_edge=It_next_edge;

          if (Direction==SEARCH_FORWARD){
            List_common_triangles.push_back(Common_triangle);
          }
          else{
            List_common_triangles.push_front(Common_triangle);
          }
        }
        else Comparted_triangle=false;
      }
    }
    else{
      Comparted_triangle=false;
      Ring=true;
      cout << "ring" << endl;
    }
  } while (Comparted_triangle==true);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::search_common_triangles(_free_point Free_point1)
{
  bool Ring=false;

  list<_edge>::iterator It_edge=Free_point1.It_edge;
  int Pos_triangle2=(*It_edge).Triangle2;
  int Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

  search_common_triangles_direction(It_edge,Pos_triangle2,Pos_edge_triangle2,SEARCH_FORWARD,Ring);
  if (Ring==false){
    search_common_triangles_direction(It_edge,Pos_triangle2,Pos_edge_triangle2,SEARCH_BACKWARD,Ring);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_vertex2f _patch::compute_projected_point(list<_point>::iterator It_point1,list<_point>::iterator It_point2,int Pos_triangle2,int Pos_edge_triangle2)
{
  _vertex2f Axis_x=(*It_point1).Patch_coordinates-(*It_point2).Patch_coordinates;

  // get the angle and lenght of the new triangle to compute the position of the new point in relation to the active edge
  float Angle=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Angles[(Pos_edge_triangle2+1)%3];

  int Vertex_1=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Vertices[(Pos_edge_triangle2+1)%3];
  int Vertex_2=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Vertices[(Pos_edge_triangle2+2)%3];

  _vertex3f Vec1=Object3D->Vertices[static_cast<unsigned long>(Vertex_2)].Position-Object3D->Vertices[static_cast<unsigned long>(Vertex_1)].Position;
  float Length_edge=float(Vec1.module());

  // now a coordinate system must be done using the coordinates of the edge
  Axis_x.normalize();
  // compute the perpendicular 90 CCW (x,y)->(-y,x)
  _vertex2f Axis_y;

  Axis_y.x=-Axis_x.y;
  Axis_y.y=Axis_x.x;
  // compute sin and cos from the angle and lenght
  float Cos,Sin;
  Cos=Length_edge*cos(Angle);
  Sin=Length_edge*sin(Angle);
  // now multiply the factor by the axis to get the new position
  Axis_x*=-Cos;
  Axis_y*=Sin;

  _vertex2f Previous_vertex2d=(*It_point1).Patch_coordinates;
  _vertex2f New_vertex2d=Axis_x+Axis_y+Previous_vertex2d;

  New_vertex2d.x=_accuracy_ns::accuracy(New_vertex2d.x);
  New_vertex2d.y=_accuracy_ns::accuracy(New_vertex2d.y);

  return(New_vertex2d);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

list<_point>::iterator
_patch::insert_new_point(_vertex2f New_vertex)
{
  _point Point;

  Point.Position=Points_counter++;
  Point.Patch_coordinates=New_vertex;
  compute_boundingbox(New_vertex);

  return(List_points.insert(List_points.end(),Point));
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::insert_new_triangles(list<_point>::iterator It_point)
{
  _triangle Triangle;
  list<_edge>::iterator It_edge1;
  list<_edge>::iterator It_edge2;
  int Pos_edge_triangle2;

  It_edge2=(*List_common_triangles.begin()).It_edge;
  for (unsigned int i=0;i<List_common_triangles.size();i++){
    It_edge1=It_edge2;
    Pos_edge_triangle2=(*It_edge1).Pos_edge_triangle2;
    It_edge2=next(List_edges,It_edge1);    

    Triangle.It_points[(Pos_edge_triangle2+1)%3]=(*It_edge1).It_point;
    Triangle.It_points[(Pos_edge_triangle2+2)%3]=It_point;
    Triangle.It_points[(Pos_edge_triangle2+3)%3]=(*It_edge2).It_point;;

    Triangle.Model_triangle=(*It_edge1).Triangle2;

    // Add the triangle
    Vector_triangles.push_back(Triangle);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::insert_new_triangle(int Pos_triangle2, int Pos_edge_triangle2, list<_point>::iterator It1, list<_point>::iterator It2, list<_point>::iterator It3)
{
  _triangle Triangle;

  Triangle.It_points[(Pos_edge_triangle2+1)%3]=It1;
  Triangle.It_points[(Pos_edge_triangle2+2)%3]=It2;
  Triangle.It_points[(Pos_edge_triangle2+3)%3]=It3;

  Triangle.Model_triangle=Pos_triangle2;

  // Add the triangle
  Vector_triangles.push_back(Triangle);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::triangles_used()
{
  list<_triangle_order>::iterator It_lto;
  list<_edge>::iterator It_edge1;
  int Pos_triangle;

  It_edge1=(*List_common_triangles.begin()).It_edge;

  for (unsigned int i=0;i<List_common_triangles.size();i++){
    Pos_triangle=(*It_edge1).Triangle2;
    It_lto=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle)].Pointer;
    Object3D->Triangles[static_cast<unsigned long>(Pos_triangle)].Pointer=list<_triangle_order>::iterator(nullptr);
    Object3D->Triangles[static_cast<unsigned long>(Pos_triangle)].Used=true;
    // remove the triangle from the list
    List_triangles_order->erase(It_lto);

    It_edge1=next(List_edges,It_edge1);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::triangle_used(int Pos_triangle)
{
  list<_triangle_order>::iterator It_lto=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle)].Pointer;
  Object3D->Triangles[static_cast<unsigned long>(Pos_triangle)].Pointer=list<_triangle_order>::iterator(nullptr);
  Object3D->Triangles[static_cast<unsigned long>(Pos_triangle)].Used=true;
  // remove the triangle from the list
  List_triangles_order->erase(It_lto);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void
_patch::insert_edge(int Pos_triangle1, int Pos_edge_triangle1, list<_edge>::iterator It_edge, list<_point>::iterator It_point)
{
  _edge Edge;

  Edge.Position=Edges_counter++;

  Edge.It_point=It_point;
  // the relative postion of the edge in the triangle is saved
  // The position of the edge in the model
  int Pos_edge=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle1)].Edges[Pos_edge_triangle1];

  // get the two triangles of the edge
  int Pos_triangle1_aux=Object3D->Edges[static_cast<unsigned long>(Pos_edge)].Triangle1;
  int Pos_triangle2_aux=Object3D->Edges[static_cast<unsigned long>(Pos_edge)].Triangle2;

  // we get the triangle that is different to the currently used
  if (Pos_triangle1_aux!=-1 && Pos_triangle1_aux!=Pos_triangle1) Edge.Triangle2=Pos_triangle1_aux;
  if (Pos_triangle2_aux!=-1 && Pos_triangle2_aux!=Pos_triangle1) Edge.Triangle2=Pos_triangle2_aux;

  if (Edge.Triangle2!=-1){
    // get the postion of the edge in the share triangle
    Edge.Pos_edge_triangle2=Object3D->Triangles_shared_edges_order[static_cast<unsigned long>(Pos_triangle1)][Pos_edge_triangle1];
  }

  // pointer to free_point
  Edge.It_free_point=List_free_points.end();

  List_edges.insert(It_edge,Edge);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::insert_front_edges_multiple(list<_point>::iterator It_new_point)
{
  int Pos_triangle;
  int Pos_edge_triangle2;
  list<_common_triangle>::iterator It_lct;
  list<_edge>::iterator It_edge;
  list<_edge>::iterator It_edge1;
  list<_edge>::iterator It_edge2;
  list<_point>::iterator It_point1;

  // where to insert
  It_lct=prev(List_common_triangles.end());
  // get the edge
  It_edge=(*It_lct).It_edge;
  It_edge=next(List_edges,It_edge);
  // first
  It_lct=List_common_triangles.begin();
  // get the edge
  It_edge1=(*It_lct).It_edge;
  It_point1=(*It_edge1).It_point;
  Pos_triangle=(*It_edge1).Triangle2;
  Pos_edge_triangle2=(*It_edge1).Pos_edge_triangle2;
  insert_edge(Pos_triangle,(Pos_edge_triangle2+1)%3,It_edge,It_point1);

  // second
  It_lct=prev(List_common_triangles.end());
  // get the edge
  It_edge2=(*It_lct).It_edge;
  Pos_triangle=(*It_edge2).Triangle2;
  Pos_edge_triangle2=(*It_edge2).Pos_edge_triangle2;

  insert_edge(Pos_triangle,(Pos_edge_triangle2+2)%3,It_edge,It_new_point);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::insert_front_edges(int Pos_triangle2, int Pos_edge_triangle2, list<_edge>::iterator It_edge, list<_point>::iterator It_point1, list<_point>::iterator It_point2)
{

  // first check that the traingles that are shared with the ned edges are not used yet
  // otherwise the new edges cannot be created  
  It_edge++;
  insert_edge(Pos_triangle2,(Pos_edge_triangle2+1)%3,It_edge,It_point1);
  insert_edge(Pos_triangle2,(Pos_edge_triangle2+2)%3,It_edge,It_point2);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::add_new_free_points(list<_edge>::iterator It_new_edge,int Num_points)
{
  // to include the new free dots
  // As two new edges are added this implies that we have to insert two new free points
  // create the free points list
  int Pos_triangle2;
  float Length_aux;
  float Area_aux;
  float Ratio_aux;
  int Pos_edge_triangle2;

  list<_edge>::iterator It_edge=It_new_edge;
  list<_free_point>::iterator It_fp;

  for (int i=0;i<Num_points;i++){
    // we get the triangle that shares the edge
    Pos_triangle2=(*It_edge).Triangle2;
    // check that there is a triangle and is not used
    if (Pos_triangle2!=-1 && Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Used==false){
      //
      Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;
      // the new edged that could be added to the front are Position+1 and Position+2
      // compute the lenght
      Length_aux=Object3D->Edges[static_cast<unsigned long>(Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Edges[(Pos_edge_triangle2+1)%3])].Edge_size;
      Length_aux+=Object3D->Edges[static_cast<unsigned long>(Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Edges[(Pos_edge_triangle2+2)%3])].Edge_size;
      Area_aux=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Area;

      Ratio_aux=Area_aux/Length_aux;

      // free point
      _free_point Free_point1;
      // the free point points to the corresponding edge
      Free_point1.Position=Free_points_counter++;
      Free_point1.It_edge=It_edge;
      Free_point1.Grade=Ratio_aux;

      list<_free_point>::iterator It_fp=List_free_points.insert(List_free_points.end(),Free_point1);
      // each edge points to the correspondin free point
      (*It_edge).It_free_point=It_fp;
      // this is for sorting depending on the grade
      _order_free_point Order_free_point;
      Order_free_point.Position=Order_free_points_counter++;
      Order_free_point.Grade=Ratio_aux;
      Order_free_point.It_free_point=It_fp;

      // order from bigger values to smaller values
      list<_order_free_point>::iterator It_ofp=List_order_free_points.begin();
      for (;It_ofp!=List_order_free_points.end();It_ofp++){
        if ((*It_ofp).Grade<Ratio_aux) break;
      }

      It_ofp=List_order_free_points.insert(It_ofp,Order_free_point);
      (*It_fp).It_order_free_point=It_ofp;
    }
    It_edge=next(List_edges,It_edge);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_vertex2f _patch::compute_mean_point()
{
  _vertex2f Mean_point;
  list<_point>::iterator It_point1;
  list<_point>::iterator It_point2;
  list<_edge>::iterator It_edge;
  list<_common_triangle>::iterator It_lct;
  int Pos_triangle_aux;
  int Pos_edge_triangle2_aux;

  // compute the mean point
  It_lct=List_common_triangles.begin();
  It_edge=(*It_lct).It_edge;

  It_point2=(*It_edge).It_point;
  for (unsigned int i=0;i<List_common_triangles.size();i++){
    It_point1=It_point2;
    Pos_triangle_aux=(*It_edge).Triangle2;
    Pos_edge_triangle2_aux=(*It_edge).Pos_edge_triangle2;

    It_edge=next(List_edges,It_edge);
    It_point2=(*It_edge).It_point;

    Mean_point=Mean_point+compute_projected_point(It_point1,It_point2,Pos_triangle_aux,Pos_edge_triangle2_aux);
  }

  Mean_point=Mean_point/List_common_triangles.size();

  Mean_point.x=_accuracy_ns::accuracy(Mean_point.x);
  Mean_point.y=_accuracy_ns::accuracy(Mean_point.y);

  return(Mean_point);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::create_deformed_triangles(_vertex2f New_point)
{
  list<_common_triangle>::iterator It_lct;
  list<_edge>::iterator It_edge;
  list<_point>::iterator It_point1;
  list<_point>::iterator It_point2;
  _triangle_deformation Triangle_deformation;
  int Pos_triangle2;
  int Pos_edge_triangle2;

  It_lct=List_common_triangles.begin();
  It_edge=(*It_lct).It_edge;
  It_point2=(*It_edge).It_point;
  for (unsigned int i=0;i<List_common_triangles.size();i++){
    It_point1=It_point2;
    Pos_triangle2=(*It_edge).Triangle2;
    Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

    It_edge=next(List_edges,It_edge);
    It_point2=(*It_edge).It_point;

    Triangle_deformation.Object3D_triangle=Pos_triangle2;
    Triangle_deformation.Vertices[(Pos_edge_triangle2+1)%3]=(*It_point1).Patch_coordinates;
    Triangle_deformation.Vertices[(Pos_edge_triangle2+2)%3]=New_point;
    Triangle_deformation.Vertices[(Pos_edge_triangle2+3)%3]=(*It_point2).Patch_coordinates;
    List_triangles_deformation.push_back(Triangle_deformation);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

float _patch::compute_triangle_deformation(_vertex2f Point0, _vertex2f Point1, _vertex2f Point2, int Pos_triangle2)
{
  _vertex3f Vec_a,Vec_b;
  float Area1,Area2;
  float Error1=0;
//  Max_error1=0;

  // compute the variation in area
  // first compute the area of the deformed triangle
  Vec_a.z=0;
  Vec_b.z=0;

  Vec_a=Point1-Point0;
  Vec_b=Point2-Point0;
  Vec_a=Vec_a.cross_product(Vec_b);
  Area1=float(Vec_a.module())/2.0f;

  Area2=Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Area;

  // compute the difference in areas as a percentage
  if (Area2>0) Error1=_accuracy_ns::accuracy(fabs(Area1-Area2)/Area2);
  else Error1=0;

  return(Error1);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

float _patch::compute_deformation(float &Mean_error1, float &Max_error1,unsigned int &Num_triangles_with_error1)
{
  _vertex3f Vec_a,Vec_b;
  float Area1,Area2;
  float Error1=0;

  Mean_error1=0;
  Max_error1=0;
  Num_triangles_with_error1=0;

  for (auto It1=List_triangles_deformation.begin();It1!=List_triangles_deformation.end();It1++){
    // compute the variation in angles  TO-DO

    // compute the variation in area
    // first compute the area of the deformed triangle
    Vec_a.z=0;
    Vec_b.z=0;

    Vec_a=(*It1).Vertices[1]-(*It1).Vertices[0];
    Vec_b=(*It1).Vertices[2]-(*It1).Vertices[0];
    Vec_a=Vec_a.cross_product(Vec_b);
    Area1=float(Vec_a.module())/2.0f;

    Area2=Object3D->Triangles[static_cast<unsigned long>((*It1).Object3D_triangle)].Area;

    if (Area2>0) Error1=_accuracy_ns::accuracy(fabs(Area1-Area2)/Area2);
    else Error1=0;

    if (Error1>0){
      if (Error1>Max_error1) Max_error1=Error1;
      Mean_error1+=Error1;
      Num_triangles_with_error1++;
    }
  }

  if (Num_triangles_with_error1>0)  return(Mean_error1/float(Num_triangles_with_error1));
  else return(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::update_errors(float Mean_error1,float Max_error1,unsigned int Num_triangles_with_error1)
{
  Mean_error+=Mean_error1;
  if (Max_error1>Max_error) Max_error=Max_error1;
  Num_triangles_with_error+=Num_triangles_with_error1;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

list<_edge>::iterator _patch::remove_edges()
{
  list<_edge>::iterator It_previous_edge;
  list<_edge>::iterator It_next_edge;
  list<_free_point>::iterator It_lfp;
  list<_order_free_point>::iterator It_lofp;

  It_next_edge=(*List_common_triangles.begin()).It_edge;
  for (unsigned int i=0;i<List_common_triangles.size();i++){
    It_previous_edge=It_next_edge;
    It_next_edge=next(List_edges,It_previous_edge);
    // get the iterator to free_point
    It_lfp=(*It_previous_edge).It_free_point;

    if (It_lfp!=List_free_points.end()){
      // get the iterator to order_free_point
      It_lofp=(*It_lfp).It_order_free_point;
      // remove them
      if (It_lofp!=List_order_free_points.end()) List_order_free_points.erase(It_lofp);
      if (It_lfp!=List_free_points.end()) List_free_points.erase(It_lfp);
    }
    List_edges.erase(It_previous_edge);
  }
  return(It_next_edge);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::remove_free_point(list<_edge>::iterator It_edge)
{
  list<_free_point>::iterator It_lfp;
  list<_order_free_point>::iterator It_lofp;

  // get the iterator to the free_point
  It_lfp=(*It_edge).It_free_point;
  if (It_lfp!=List_free_points.end()){
    // get the iterator to order_free_point
    It_lofp=(*It_lfp).It_order_free_point;
    // remove them
    List_order_free_points.erase(It_lofp);
    List_free_points.erase(It_lfp);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool _patch::add_single_point(_free_point Free_point1)
{
  list<_edge>::iterator It_edge=Free_point1.It_edge;
  int Pos_triangle2=(*It_edge).Triangle2;
  int Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

  list<_point>::iterator It_point1;
  list<_point>::iterator It_point2;
  list<_point>::iterator It_point12;

  It_point1=(*It_edge).It_point;
  It_point2=(*next(List_edges,It_edge)).It_point;

  _vertex2f New_point=compute_projected_point(It_point1,It_point2,Pos_triangle2,Pos_edge_triangle2);

  // to check if there is an intersection with the front
  _vertex2f Previous_vertex2d=(*It_point1).Patch_coordinates;
  _vertex2f Next_vertex2d=(*It_point2).Patch_coordinates;

  if (check_intersection_with_front(Previous_vertex2d,New_point,Next_vertex2d)==false){
    // there is no intersections
    // get the pointer to the first point of the active edge
    It_point1=(*It_edge).It_point;
    // get the point to the second point of the active edge
    It_point2=(*next(List_edges,It_edge)).It_point;
    // create and insert the new point
    It_point12=insert_new_point(New_point);
    // create the triangle
    insert_new_triangle(Pos_triangle2,Pos_edge_triangle2,It_point1,It_point12,It_point2);
    // remove the triangle for List_triangles_order
    if (Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Intersected){
//      cout << "triangle was intersected but now is included" << endl;
      Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Intersected=false;
    }

    triangle_used(Pos_triangle2);

    // insert the new edges
    insert_front_edges(Pos_triangle2,Pos_edge_triangle2,It_edge,It_point1,It_point12);
    // remove the free_point
    remove_free_point(It_edge);
    // remove the edge
    list<_edge>::iterator It_new_edge1=List_edges.erase(It_edge);
    // to include the new free dots and compute the ratios
    add_new_free_points(It_new_edge1,2);
    return true;
  }
  return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::add_free_point(_free_point Free_point1)
{
  // search in the edge list the edge that contains the triangle. it is important
  // to include the new edges if the triangle is acepted
  list<_edge>::iterator It_edge=Free_point1.It_edge;

  float Mean_error1;
  float Max_error1;
  unsigned int Num_triangles_with_error1;

//  int Active_edge=(*It_edge).Position;
//  cout << "active Edge=" << Active_edge << endl;

//  for (auto It=List_edges.begin();It!=List_edges.end();It++){
//    cout << (*It).Position << " ";
//  }
//  cout << endl;


  int Pos_triangle2=(*It_edge).Triangle2;

  // check that the triangle to include has not been used previously
  if (Object3D->Triangles[static_cast<unsigned long>(Pos_triangle2)].Used==true){
    remove_free_point(It_edge);
    return;
  }

  // the coordinates of the triangle/s must be computed and adapted to be located
  // in the shared edge
  // search common triangles
  List_common_triangles.clear();
  _common_triangle Common_triangle;

  Common_triangle.It_edge=It_edge;
  List_common_triangles.push_back(Common_triangle);

  // search for common triangles
  Same_triangle=false;
  search_common_triangles(Free_point1);

  if (Same_triangle==false && List_common_triangles.size()>1){
    list<_point>::iterator It_point1;
    list<_point>::iterator It_point2;
    list<_point>::iterator It_point12;
    _vertex2f New_point;

    // compute the projections of the common triangles
    New_point=compute_mean_point();

     // check that mean_point is outside the simple front
    if (check_point_is_outside_front(New_point)==true){
      // the point is outside the front -> OK

      // create the deformed triangles
      List_triangles_deformation.clear();
      create_deformed_triangles(New_point);
      // compute the deformation
      float Sum_difference_areas=compute_deformation(Mean_error1,Max_error1,Num_triangles_with_error1);

      if (Sum_difference_areas>Distortion){
        // not possible to flat all the common triangles
        if (add_single_point(Free_point1)==false){
          // remove the free point
          remove_free_point(It_edge);
          //update to "NULL" the pointer in edge
          (*It_edge).It_free_point=List_free_points.end();
        }
      }
      else{      
        // Possible to flat all the common triangles

        // add the computed errors
        update_errors(Mean_error1,Max_error1,Num_triangles_with_error1);

        // first check that there is not intersection with the front
        // take the first edge
        list<_edge>::iterator It_edge=(*List_common_triangles.begin()).It_edge;
        list<_edge>::iterator It_edge1;

//        // check that two edges point to the same triangle
//        if ((*It_edge).Triangle2==(*next(It_edge)).Triangle2){
//          cout << "same triangle " << endl;
//        }


        // and its point
        It_point1=(*It_edge).It_point;
        // now advance in the front to one passed the last edge
        It_edge1=(*prev(List_common_triangles.end())).It_edge;
        It_edge1=next(List_edges,It_edge1);

        if (It_edge!=It_edge1){ // the points are differents
          // get the last point
          It_point2=(*It_edge1).It_point;
          // to check if there is an intersection with the front
          _vertex2f First_point=(*It_point1).Patch_coordinates;
          _vertex2f Last_point=(*It_point2).Patch_coordinates;

          if (check_intersection_with_front(First_point,New_point,Last_point)==false){

            // add the mean point
            It_point12=insert_new_point(New_point);
            // add the triangles
            insert_new_triangles(It_point12);
            // remove the trianglea for List_triangles_order
            triangles_used();
            // add the two edges to the front
            insert_front_edges_multiple(It_point12);
            // remove the old edges of the front
            list<_edge>::iterator It_new_edge1=remove_edges();
            // to include the new free dots and compute the ratios
            add_new_free_points(It_new_edge1,2);
          }
          else{
            // the mean point intersects with the front. Try a single triangle
            if (add_single_point(Free_point1)==false){
              // remove the free point
              It_edge=Free_point1.It_edge;

              remove_free_point(It_edge);
              //update to "NULL" the pointer in edge
              (*It_edge).It_free_point=List_free_points.end();
            }
          }
        }
        else{
          cout << "There is a ring: the first and last points are the same" << endl;
          exit(-1);
          // to include the point, the triangles but not add edges and free points. The front is removed
        }
      }
    }
    else{
      // The mean point is inside the simple front-> try to add only one triangle
      if (add_single_point(Free_point1)==false){
        // remove the free point
        remove_free_point(It_edge);
        //update to "NULL" the pointer in edge
        (*It_edge).It_free_point=List_free_points.end();
      }
    }
  }
  else{
    // SINGLE
    if (Same_triangle==true){
      if (List_common_triangles.size()==2){
        // 2 common
        // compute the positions of the new triangle
        list<_edge>::iterator It_edge=(*List_common_triangles.begin()).It_edge;
        list<_point>::iterator It_point0;
        list<_point>::iterator It_point1;
        list<_point>::iterator It_point2;

        // the position of the shared triangle
        Pos_triangle2=(*It_edge).Triangle2;
        int Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

        // get the points
        It_point0=(*It_edge).It_point;
        It_edge=next(List_edges,It_edge);
        It_point1=(*It_edge).It_point;
        It_edge=next(List_edges,It_edge);
        It_point2=(*It_edge).It_point;

        _vertex2f Point0=(*It_point0).Patch_coordinates;
        _vertex2f Point1=(*It_point1).Patch_coordinates;
        _vertex2f Point2=(*It_point2).Patch_coordinates;

        float Triangle_distortion=compute_triangle_deformation(Point0,Point2,Point1,Pos_triangle2);

        if (Triangle_distortion<=Distortion){
          // to check if there is an intersection with the front
          if (check_intersection_with_front(Point0,Point2,Point1)==false){
            // add error
            update_errors(Triangle_distortion,Triangle_distortion,1);

            insert_new_triangle(Pos_triangle2,Pos_edge_triangle2,It_point0,It_point2,It_point1);
            // remove the triangle for List_triangles_order
            triangle_used(Pos_triangle2);

            // insert the new edge
            // get the last edge
            It_edge=(*(--List_common_triangles.end())).It_edge;
            // nove to the next
            It_edge=next(List_edges,It_edge);
            insert_edge(Pos_triangle2,(Pos_edge_triangle2+1)%3,It_edge,It_point0);

            // remove the edges
            list<_edge>::iterator It_new_edge1=remove_edges();
            //
            add_new_free_points(It_new_edge1,1);
          }
          else{// cannot include the two side triangle. Try one
            if (add_single_point(Free_point1)==false){
              // remove the free point
              It_edge=Free_point1.It_edge;
              remove_free_point(It_edge);
              //update to "NULL" the pointer in edge
              (*It_edge).It_free_point=List_free_points.end();
            }
          }
        }
        else{
          if (add_single_point(Free_point1)==false){
            // remove the free point
            It_edge=Free_point1.It_edge;
            remove_free_point(It_edge);
            //update to "NULL" the pointer in edge
            (*It_edge).It_free_point=List_free_points.end();
          }
        }
      }
      else{ // 3 common
        // there are 3 posibilities
        // compute the positions of the new triangle
        It_edge=(*List_common_triangles.begin()).It_edge;
        list<_point>::iterator Vec_it_point[4];
        _vertex2f Vec_points[4];

        // the position of the shared triangle
        Pos_triangle2=(*It_edge).Triangle2;
        int Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

        // get the points
        for (int i=0;i<4;i++){
          Vec_it_point[i]=(*It_edge).It_point;
          It_edge=next(List_edges,It_edge);
          Vec_points[i]=(*Vec_it_point[i]).Patch_coordinates;
        }

        // now check the two triangles
        int Pos;
        float Deformation;
        float Deformations[2];

        Deformations[0]=-1;
        Deformations[1]=-1;

        for (int i=0;i<2;i++){
          // to check if there is an intersection with the front
          if (check_intersection_with_front(Vec_points[i],Vec_points[i+2],Vec_points[i+1])==false){
            Deformation=compute_triangle_deformation(Vec_points[i],Vec_points[i+2],Vec_points[i+1],Pos_triangle2);
            if (Deformation<=Distortion) Deformations[i]=Deformation;
          }
        }
        if (Deformations[0]!=-1 || Deformations[1]!=-1){
          // it possible to deform
          if (Deformations[0]!=-1 && Deformations[1]!=-1){
            if (Deformations[0]<=Deformations[1]) Pos=0;
            else Pos=1;
          }
          else{
            if (Deformations[0]!=-1) Pos=0;
            else Pos=1;
          }

          if (Pos==0){
            // add error
            update_errors(Deformations[0],Deformations[0],1);

            It_edge=(*List_common_triangles.begin()).It_edge;
            Pos_triangle2=(*It_edge).Triangle2;
            Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

            insert_new_triangle(Pos_triangle2,Pos_edge_triangle2,Vec_it_point[0],Vec_it_point[2],Vec_it_point[1]);
            triangle_used(Pos_triangle2);
            // advance 2
            It_edge=next(List_edges,It_edge,2);
            insert_edge(Pos_triangle2,(Pos_edge_triangle2+1)%3,It_edge,Vec_it_point[0]);
            // remove the las position becasue we won't to delete the edge
            List_common_triangles.pop_back();
          }
          else{// it is necessary to move the edge one step
            // add error
            update_errors(Deformations[1],Deformations[1],1);

            It_edge=(*List_common_triangles.begin()).It_edge;
            It_edge=next(List_edges,It_edge);
            Pos_triangle2=(*It_edge).Triangle2;
            Pos_edge_triangle2=(*It_edge).Pos_edge_triangle2;

            insert_new_triangle(Pos_triangle2,Pos_edge_triangle2,Vec_it_point[1],Vec_it_point[3],Vec_it_point[2]);
            triangle_used(Pos_triangle2);
            // advance 3
            It_edge=(*List_common_triangles.begin()).It_edge;
            It_edge=next(List_edges,It_edge,3);
            insert_edge(Pos_triangle2,(Pos_edge_triangle2+1)%3,It_edge,Vec_it_point[1]);
            // remove the first position becasue we won't to delete the edge
            List_common_triangles.pop_back();
          }
          // remove the edges
          list<_edge>::iterator It_new_edge1=remove_edges();
          //
//          add_new_free_points(It_new_edge1,1);
        }
        else{
          if (add_single_point(Free_point1)==false){
            // remove the free point
            It_edge=Free_point1.It_edge;
            remove_free_point(It_edge);
            //update to "NULL" the pointer in edge
            (*It_edge).It_free_point=List_free_points.end();
          }
        }
      }
    }
    else{
      if (add_single_point(Free_point1)==false){
        // remove the free point
        remove_free_point(It_edge);
        //update to "NULL" the pointer in edge
        (*It_edge).It_free_point=List_free_points.end();
      }
    }
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool _patch::check_intersection_with_front(_vertex2f Vertex1,_vertex2f Vertex2,_vertex2f Vertex3)
{
  // compute the intersection between one of the edges of the added point and the front border
  // it is made as the computation of the intersection of two lines and check the value of the parameter
  // if it is between 0 and 1 then there is an intersection and the point must be rejected

  // This computation could be made drawing the triangles or using a quadtree
  _vertex2f Vertex1_aux,Vertex2_aux;
  list<_edge>::iterator It_edge=List_edges.begin();
  list<_point>::iterator It_point;

  // for each edge of the front the vertices are taken
  It_point=(*It_edge).It_point;
  Vertex1_aux=(*It_point).Patch_coordinates;
  for (unsigned int i=0;i<List_edges.size();i++){
    It_edge=next(List_edges,It_edge);
    It_point=(*It_edge).It_point;
    Vertex2_aux=(*It_point).Patch_coordinates;
    // check that the points are different
    if (Vertex1!=Vertex1_aux && Vertex1!=Vertex2_aux && Vertex2!=Vertex1_aux && Vertex2!=Vertex2_aux){
      if (compute_two_lines_intersection(Vertex1,Vertex2,Vertex1_aux,Vertex2_aux)==1) return true;
    }
    // check that the points are different
    if (Vertex2!=Vertex1_aux && Vertex2!=Vertex2_aux && Vertex3!=Vertex1_aux && Vertex3!=Vertex2_aux){
      if (compute_two_lines_intersection(Vertex2,Vertex3,Vertex1_aux,Vertex2_aux)==1) return true;
    }

    Vertex1_aux=Vertex2_aux;
  }
  return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

int _patch::compute_two_lines_intersection(_vertex2f Vertex1,_vertex2f Vertex2,_vertex2f Vertex3,_vertex2f Vertex4)
{
  // compute the intersection between two lines in parametric form
  // the s and t parameters of the intersections are computed
  // The intersection is valid if 0<=s<=1 and 0<=t<=1

  float A=0,B=0,C=0;
  float s=-1,t=-1;
  _vertex2f V1,V2;

  V1=Vertex2-Vertex1;
  V2=Vertex4-Vertex3;

  A=Vertex3.x-Vertex1.x;
  B=Vertex3.y-Vertex1.y;

  C=V2.y*V1.x-V2.x*V1.y;

  if (C!=0){
    s=(V1.y*A-V1.x*B)/C;
    if (s>=0.0f && s<=1.0f){
      if (V1.x!=0){
        t=(A+V2.x*s)/V1.x;
        if (t>=0.0f && t<=1.0f) return 1; //intersection
      }
      else return 0; // no intersection
    }
    else return 0; // no intersection
  }

  return 2; // the parallel lines
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool _patch::check_point_is_outside_front(_vertex2f New_vertex2d)
{
  // This computation could be made drawing the triangles or using a quadtree
  _vertex2f Vertex1_aux,Vertex2_aux;
  list<_common_triangle>::iterator It_lct;
  list<_edge>::iterator It_edge;
  list<_point>::iterator It_point1;
  list<_point>::iterator It_point2;
  _vertex3f Vec_a,Vec_b;
  bool Outside=true;

  It_lct=List_common_triangles.begin();
  It_edge=(*It_lct).It_edge;
  It_point2=(*It_edge).It_point;
  Vertex2_aux=(*It_point2).Patch_coordinates;

  for (unsigned int i=0;i<List_common_triangles.size();i++){
    Vertex1_aux=Vertex2_aux;

    It_edge=next(List_edges,It_edge);
    It_point2=(*It_edge).It_point;
    Vertex2_aux=(*It_point2).Patch_coordinates;

    // compute the two vectors
    Vec_a.z=0;

    Vec_a=New_vertex2d-Vertex1_aux;
    Vec_b=Vertex2_aux-Vertex1_aux;

    // normalize
    Vec_a.normalize();
    Vec_b.normalize();
    // cross product
    Vec_a=Vec_a.cross_product(Vec_b);
    if (Vec_a.z>=0){
      // outside
      continue;
    }
    else{
      // inside
      Outside=false;
      break;
    }
//    Vertex1_aux=Vertex2_aux;
  }

  return Outside;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::compute_boundingbox(_vertex2f Point)
{
  if (Point.x<Min_position.x) Min_position.x=Point.x;
  if (Point.y<Min_position.y) Min_position.y=Point.y;
  if (Point.x>Max_position.x) Max_position.x=Point.x;
  if (Point.y>Max_position.y) Max_position.y=Point.y;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _patch::compute_area_dimensions()
{
  Dimension=Max_position-Min_position;
  Area=fabs(Dimension.x*Dimension.y);
}
