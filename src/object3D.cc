/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */

#pragma GCC diagnostic ignored "-Wconversion"

#include "object3D.h"
#include <QString>

#include "parametrizador.h"

using namespace std;
using namespace _object3D_ns;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool compare_angle(const float &First, const float& Second)
{
  // order: first greater values
  if (First>Second) return true;
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_object3D::_object3D()
{
  Bounding_box=false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_object3D::~_object3D()
{
  Vertices.clear();
  Triangles.clear();
  Edges.clear();

  Projected_vertices.clear();
  Projected_triangles.clear();

  Triangles_shared_edges_order.clear();

  Vertices_colors.clear();
  Triangles_colors.clear();
  Vertices_texture_coordinates.clear();

  Vertices_drawarray.clear();
  Vertices_colors_drawarray.clear();
  Vertices_triangles_colors_drawarray.clear();
  Vertices_normals_drawarray.clear();
  Vertices_triangles_normals_drawarray.clear();
  Vertices_texture_coordinates_drawarray.clear();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::create(vector<_vertex3f> &Vertices1, vector<_vertex3i> &Triangles1)
{
  _vertex3f Vertex;

  int Pos_edge = 0;
  int Pos_edge_aux=0;
  bool Exist;
  _vertex_edge Vertex_edge;
  int Lower_position,Higher_position;
  _vertex3f Vec_a,Vec_b;
//  float Area1;

  Min_position(1e10,1e10,1e10);
  Max_position(1e-10f,1e-10f,1e-10f);

  Min_value=1e10f;
  Max_value=0;

  Vertices.resize(Vertices1.size());
  Triangles.resize(Triangles1.size());

  vector<vector<_vertex_edge>> Vec_vertex_edge;
  Vec_vertex_edge.resize(Vertices.size());

  // El numero de aristas es: |E| = 3*|F|/2  (E=aristas,F=Caras)
  int Num_edges = 3*int(Triangles.size());
  Edges.resize(Num_edges);

  // Initialize the vertices
  for (unsigned int i=0;i<Vertices.size();i++){
    Vertex=Vertices1[i];

    Vertices[i].Position=Vertex;

    // compute min max
    if (Vertex.x<Min_position.x) Min_position.x=Vertex.x;
    if (Vertex.y<Min_position.y) Min_position.y=Vertex.y;
    if (Vertex.z<Min_position.z) Min_position.z=Vertex.z;
    if (Vertex.x>Max_position.x) Max_position.x=Vertex.x;
    if (Vertex.y>Max_position.y) Max_position.y=Vertex.y;
    if (Vertex.z>Max_position.z) Max_position.z=Vertex.z;

    if (fabs(Vertex.x)<Min_value) Min_value=fabs(Vertex.x);
    if (fabs(Vertex.y)<Min_value) Min_value=fabs(Vertex.y);
    if (fabs(Vertex.z)<Min_value) Min_value=fabs(Vertex.z);
    if (fabs(Vertex.x)>Max_value) Max_value=fabs(Vertex.x);
    if (fabs(Vertex.y)>Max_value) Max_value=fabs(Vertex.y);
    if (fabs(Vertex.z)>Max_value) Max_value=fabs(Vertex.z);

    // Normal
    Vertices[i].Normal = _vertex3f(0,0,0);
  }

  Bounding_box=true;

  Center_position=Max_position+Min_position;
  Center_position/=2;

  // centers and scales the object
  cout << "Centering and scaling model" << endl;

  float Factor;

  if (Min_value==0.0f) Factor=1.0f;
  else Factor=roundf(fabs(log10(Min_value)));

  Factor=powf(10,Factor/2);

  for (unsigned int i=0;i<Vertices.size();i++){
    Vertices[i].Position=(Vertices[i].Position-Center_position)*Factor;
  }

  Min_position*=Factor;
  Max_position*=Factor;

  _vertex3f Vertices_aux[3];
  _vertex3f Vector0,Vector1,Vector2;
  _vertex3f Normal;

  for (unsigned int i=0;i<Triangles.size();i++){
    Triangles[i].Vertices=Triangles1[i];

    // Indices
    Vertices_aux[0] = Vertices[Triangles1[i]._0].Position;
    Vertices_aux[1] = Vertices[Triangles1[i]._1].Position;
    Vertices_aux[2] = Vertices[Triangles1[i]._2].Position;

    // Vectors
    Vector0 = (Vertices_aux[1] - Vertices_aux[0]).normalize(); // 0-1
    Vector1 = (Vertices_aux[2] - Vertices_aux[1]).normalize(); // 1-2
    Vector2 = (Vertices_aux[0] - Vertices_aux[2]).normalize(); // 2-0

    // Angles
    Triangles[i].Angles[0] = acosf(Vector0.dot_product(Vector2*-1.0));
    Triangles[i].Angles[1] = acosf(Vector1.dot_product(Vector0*-1.0));
    Triangles[i].Angles[2] = acosf(Vector2.dot_product(Vector1*-1.0));

     // Triangle normal
    Normal = Vector0.cross_product(Vector1);
    Normal.normalize();
    Triangles[i].Normal=Normal;

    // Add the triangle to each vertex of the triangle
    for(unsigned int j = 0; j < 3; j++){
      Vertices[Triangles[i].Vertices[j]].Triangles.push_back(i);
    }

    // for each edge
    for (unsigned int k=0;k<3;k++){
      Triangles[i].Edges[k]=-1;
      // check that is not a degenerated edge
      if(Triangles[i].Vertices[k] != Triangles[i].Vertices[(k+1)%3]){
        // one edge is composed by two vertices and always the index on one vertex is less than the other
        // the initial vertex of an edge is always that with lower index
        if (Triangles[i].Vertices[k]<Triangles[i].Vertices[(k+1)%3]){
          Lower_position=Triangles[i].Vertices[k];
          Higher_position=Triangles[i].Vertices[(k+1)%3];
        }
        else{
          Lower_position=Triangles[i].Vertices[(k+1)%3];
          Higher_position=Triangles[i].Vertices[k];
        }
        Exist = false;
        // check if there is an edge with the same vertex
        for(unsigned int j = 0; j < Vec_vertex_edge[Lower_position].size(); j++){
          // Check if the second vertex of the new edge is equal to the first vertex of an included edge
          if(Vec_vertex_edge[Lower_position][j].Vertex == Higher_position){
            // if the edge is include the second triangle of the edge must be updated
            Pos_edge_aux = Vec_vertex_edge[Lower_position][j].Edge;
            Edges[Pos_edge_aux].Triangle2 = i;
            Exist = true;
            break;
          }
        }

        // The new edge must be included
        if(!Exist){
          // to check if the edge is included it must be put for each vertex as initial vertex
          Vertex_edge.Vertex=Higher_position;
          Vertex_edge.Edge=Pos_edge;
          Vec_vertex_edge[Lower_position].push_back(Vertex_edge);

          // create the edge
          Edges[Pos_edge].Vertex1 = Lower_position;
          Edges[Pos_edge].Vertex2 = Higher_position;
          Edges[Pos_edge].Triangle1 = i;
          Edges[Pos_edge].Triangle2 = -1;
          Edges[Pos_edge].Edge_size = (Vertices_aux[(k+1)%3]-Vertices_aux[k]).module();

          Pos_edge_aux = Pos_edge;
          Pos_edge++;
        }
        Triangles[i].Edges[k] = Pos_edge_aux;
      }
      else{
        cout << "The model has a degenerated triangle (" << i << ")" << endl;
        exit(-1);
      }
    }

    // Área de la cara
    float base = Edges[ Triangles[i].Edges[0] ].Edge_size;
    float altura = Edges[ Triangles[i].Edges[2] ].Edge_size* sin( Triangles[i].Angles[0]);
    Triangles[i].Area = base *altura /2;

    if(Triangles[i].Area > Greater_area) Greater_area = Triangles[i].Area;
  }
  // There is not coincidence with Euler characteristic V-E+F=2 ¿?
  Edges.resize(Pos_edge);

  compute_vertices_normals();

  compute_projected_triangles();

  compute_shared_edges_order();

  compute_vertices_angles();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::create_data_drawarrays()
{
  // for each triangle
  Vertices_drawarray.resize(Triangles.size()*3);
  Vertices_colors_drawarray.resize(Triangles.size()*3);
  Vertices_triangles_colors_drawarray.resize(Triangles.size()*3);
  Vertices_normals_drawarray.resize(Triangles.size()*3);
  Vertices_triangles_normals_drawarray.resize(Triangles.size()*3);
  Vertices_texture_coordinates_drawarray.resize(Triangles.size()*3);

  random_device RD;
  mt19937 MT(RD());
  uniform_real_distribution<float> Random(0.0f,0.9f);
  _vertex3f Color;

  Vertices_colors.resize(Vertices.size());
  for (unsigned int i=0;i<Vertices_colors.size();i++){
    Color.x=Random(MT);
    Color.y=Random(MT);
    Color.z=Random(MT);
    Vertices_colors[i]=Color;
  }

  Triangles_colors.resize(Triangles.size());
  for (unsigned int i=0;i<Triangles_colors.size();i++){
    Color.x=Random(MT);
    Color.y=Random(MT);
    Color.z=Random(MT);
    Triangles_colors[i]=Color;
  }

  for (unsigned int i=0;i<Triangles.size();i++){
    // fill the vertices
    Vertices_drawarray[i*3]=Vertices[Triangles[i].Vertices[0]].Position;
    Vertices_drawarray[i*3+1]=Vertices[Triangles[i].Vertices[1]].Position;
    Vertices_drawarray[i*3+2]=Vertices[Triangles[i].Vertices[2]].Position;
    // fill the verices colors
    Vertices_colors_drawarray[i*3]=Vertices_colors[Triangles[i].Vertices[0]];
    Vertices_colors_drawarray[i*3+1]=Vertices_colors[Triangles[i].Vertices[1]];
    Vertices_colors_drawarray[i*3+2]=Vertices_colors[Triangles[i].Vertices[2]];
    // fill the verices colors
    Vertices_triangles_colors_drawarray[i*3]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+1]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+2]=Triangles_colors[i];
    // fill the normals
    Vertices_normals_drawarray[i*3]=Vertices[Triangles[i].Vertices[0]].Normal;
    Vertices_normals_drawarray[i*3+1]=Vertices[Triangles[i].Vertices[1]].Normal;
    Vertices_normals_drawarray[i*3+2]=Vertices[Triangles[i].Vertices[2]].Normal;
    // fill the normals of triangles
    Vertices_triangles_normals_drawarray[i*3]=Triangles[i].Normal;
    Vertices_triangles_normals_drawarray[i*3+1]=Triangles[i].Normal;
    Vertices_triangles_normals_drawarray[i*3+2]=Triangles[i].Normal;
    // fill the texture coordinates
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::update_texture_data_drawarrays(_parametrizer *Parametrizer1)
{
  list<_parametrizer_ns::_point>::iterator It_p;
  int Model_triangle;

  for(unsigned int Patch = 0; Patch < Parametrizer1->Vector_patches.size(); Patch++){
//  for(unsigned int Patch = 0; Patch < 1; Patch++){
    for(unsigned int Triangle = 0; Triangle < Parametrizer1->Vector_patches[Patch]->Vector_triangles.size();Triangle++){
      Model_triangle=Parametrizer1->Vector_patches[Patch]->Vector_triangles[Triangle].Model_triangle;
      for (int i=0;i<3;i++){
        It_p=Parametrizer1->Vector_patches[Patch]->Vector_triangles[Triangle].It_points[i];
//        cout << "Texture coordinates t=" << Triangle << " V=" << i << endl;
//        (*It_p).Texture_coordinates.show_values();
//        Vertices_drawarray[Model_triangle*3+i].show_values();

        Vertices_texture_coordinates_drawarray[Model_triangle*3+i]=(*It_p).Texture_coordinates;

      }

     Triangles_colors[Model_triangle]=Parametrizer1->Vector_patches[Patch]->Color;
    }
  }

  // pass the color of triangles to the vertices
  for (unsigned int i=0;i<Triangles.size();i++){
    // fill the colors
    Vertices_triangles_colors_drawarray[i*3]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+1]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+2]=Triangles_colors[i];
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::draw()
{
  glDrawArrays(GL_TRIANGLES,0,Triangles.size()*3);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

float _object3D::max_dimension()
{
  if (Bounding_box){
    _vertex3f Dimensions=Max_position-Min_position;
    float Max=Dimensions.x;
    if (Dimensions.y>Max) Max=Dimensions.y;
    if (Dimensions.z>Max) Max=Dimensions.z;

    return(Max);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::compute_spherical_texture_coordinates()
{
  _vertex3f V1;

  Vertices_texture_coordinates.resize(Vertices.size());
  for (unsigned int i=0;i<Vertices.size();i++){
    V1=Vertices[i].Position;
    V1.normalize();
    Vertices_texture_coordinates[i].s=asinf(V1.x)/M_PI+0.5f;
    Vertices_texture_coordinates[i].t=asinf(V1.y)/M_PI+0.5f;
  }

  // update the drawarray
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_texture_coordinates_drawarray[i*3]=Vertices_texture_coordinates[Triangles[i].Vertices[0]];
    Vertices_texture_coordinates_drawarray[i*3+1]=Vertices_texture_coordinates[Triangles[i].Vertices[1]];
    Vertices_texture_coordinates_drawarray[i*3+2]=Vertices_texture_coordinates[Triangles[i].Vertices[2]];
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::compute_vertices_normals()
{
  int Pos;
  vector<_vertex3f> Vertices_normals;
  vector<int> Num_normals;

  Vertices_normals.resize(Vertices.size());
  Num_normals.resize(Vertices.size());

  for (unsigned int i=0;i<Triangles.size();i++){
    for (unsigned int j=0;j<3;j++){
      Pos=Triangles[i].Vertices[j];
      Vertices_normals[Pos]+=Triangles[i].Normal;
      Num_normals[Pos]++;
    }
  }
  for (unsigned int i=0;i<Vertices.size();i++){
    Vertices_normals[i]/=Num_normals[i];
    Vertices[i].Normal=Vertices_normals[i].normalize();
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::compute_projected_triangles()
{
  Projected_triangles.resize(Triangles.size());
  Projected_vertices.resize(Triangles.size()*3);

  // to construct the transformation to change from the Coordinate System of the triangle to the World CS
  // Given a triangle V0,V1,V2 plus the normal N
  // The local coordinate system is
  // Z=N
  // X=V1-V0
  // Y=ZxX
  // We must do the translation of Vertex0 to the origin

  QMatrix4x4 Mat1,Mat2;
  _vertex3f Aux,Aux1,Vertex1,Vertex2;
  _vertex2f Point1,Point2;

  for (unsigned int Triangle1=0;Triangle1<Triangles.size();Triangle1++){
    Aux=Triangles[Triangle1].Normal;
    QVector4D Vector_z(Aux.x,Aux.y,Aux.z,0);

    Vertex1=Vertices[Triangles[Triangle1].Vertices[0]].Position;
    Vertex2=Vertices[Triangles[Triangle1].Vertices[1]].Position;
    Aux1=Vertex2-Vertex1;
    Aux1.normalize();
    QVector4D Vector_x(Aux1.x,Aux1.y,Aux1.z,0);

    Vertex2=Aux.cross_product(Aux1);
    Vertex2.normalize();
    QVector4D Vector_y(Vertex2.x,Vertex2.y,Vertex2.z,0);

    Mat1.setToIdentity();
    // The inverse is obtained by putting the vectors as rows
    Mat1.setRow(0,Vector_x);
    Mat1.setRow(1,Vector_y);
    Mat1.setRow(2,Vector_z);

    Mat2.setToIdentity();
    Mat2.translate(-Vertex1.x,-Vertex1.y,-Vertex1.z);
    Mat1*=Mat2;

    Vertex1=Vertices[Triangles[Triangle1].Vertices[1]].Position;
    QVector4D Vertex4D_1(Vertex1.x,Vertex1.y,Vertex1.z,1);

    Vertex4D_1=Mat1*Vertex4D_1;
    Point1.x=Vertex4D_1.x();
    Point1.y=Vertex4D_1.y();

    Vertex2=Vertices[Triangles[Triangle1].Vertices[2]].Position;
    QVector4D Vertex4D_2(Vertex2.x,Vertex2.y,Vertex2.z,1);

    Vertex4D_2=Mat1*Vertex4D_2;
    Point2.x=Vertex4D_2.x();
    Point2.y=Vertex4D_2.y();

    // the first point is at (0,0)
    Projected_vertices[Triangle1*3]=_vertex2f(0,0);
    // the second point is located in axis X  at is size
    Projected_vertices[Triangle1*3+1]=Point1;
    // the third point is in plane z=0
    Projected_vertices[Triangle1*3+2]=Point2;

    Projected_triangles[Triangle1]=_vertex3i(Triangle1*3,Triangle1*3+1,Triangle1*3+2);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::compute_shared_edges_order()
{
  int Triangle1;
  int Triangle2;
  bool Found;
  int i,j;

  Triangles_shared_edges_order.resize(Triangles.size());

  for (unsigned int i=0;i<Triangles_shared_edges_order.size();i++){
    Triangles_shared_edges_order[i]=_vertex3i(-1,-1,-1);
  }

  for (unsigned int Edge1=0;Edge1<Edges.size();Edge1++){
    Found=false;

    Triangle1=Edges[Edge1].Triangle1;
    Triangle2=Edges[Edge1].Triangle2;
    if (Triangle1!=-1 && Triangle2!=-1){
      // check the coincidence
      for (i=0;i<3;i++){
        for (j=0;j<3;j++){
          if (Triangles[Triangle1].Vertices[i]==Triangles[Triangle2].Vertices[(j+1)%3] && Triangles[Triangle1].Vertices[(i+1)%3]==Triangles[Triangle2].Vertices[j]){
            Found=true;
            break;
          }
        }
        if (Found) break;
      }
      // save the data
      Triangles_shared_edges_order[Triangle1][i]=j;
      Triangles_shared_edges_order[Triangle2][j]=i;
    }
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::get_triangles(int Selected_triangle, int &Selected_triangle1, int &Selected_triangle2, int &Selected_triangle3)
{
  int Vec_triangles[3];

  int Edge_aux;
  for (int i=0;i<3;i++){
    Edge_aux=Triangles[Selected_triangle].Edges[i];
    if (Edges[Edge_aux].Triangle1!=Selected_triangle) Vec_triangles[i]=Edges[Edge_aux].Triangle1;
    else Vec_triangles[i]=Edges[Edge_aux].Triangle2;
  }

  Selected_triangle1=Vec_triangles[0];
  Selected_triangle2=Vec_triangles[1];
  Selected_triangle3=Vec_triangles[2];
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _object3D::compute_vertices_angles()
{
  int Triangle;
  int k;
  vector<float> Vertices_angles;

  Vertices_angles.resize(Vertices.size());

  for (unsigned int i=0;i<Vertices.size();i++){
    Vertices_angles[i]=0;
    for (unsigned int j=0;j<Vertices[i].Triangles.size();j++){
      Triangle=Vertices[i].Triangles[j];
      for (k=0;k<3;k++){
        if (Triangles[Triangle].Vertices[k]==(int)i) break;
      }
      Vertices_angles[i]+=Triangles[Triangle].Angles[k];
    }
  }

  float Angle;
  for (unsigned int i=0;i<Triangles.size();i++){
    Angle=0;
    for (int j=0;j<3;j++){
      Angle+=Vertices_angles[Triangles[i].Vertices[j]];
    }
    Triangles[i].Angle=Angle/3;
  }
}
