/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "texture_object.h"
#include "object3D.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_texture_object::_texture_object()
{
  Initial_position=0;
  Num_vertices=0;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _texture_object::create(_object3D *Ply_object)
{
  Vertices.resize(Ply_object->Vertices.size());
  Vertices_texture_coordinates.resize(Vertices.size());
  Vertices_normals.resize(Vertices.size());

  for (unsigned int i=0;i<Vertices.size();i++){
    Vertices[i]=Ply_object->Vertices_texture_coordinates[i];
    Vertices_texture_coordinates[i]=Ply_object->Vertices_texture_coordinates[i];
    Vertices_normals[i]=_vertex3f(0,0,1);
  }

  Triangles.resize(Ply_object->Triangles.size());
  Triangles_normals.resize(Triangles.size());

  for (unsigned int i=0;i<Triangles.size();i++){
    Triangles[i]._0=Ply_object->Triangles[i].Vertices[0];
    Triangles[i]._1=Ply_object->Triangles[i].Vertices[1];
    Triangles[i]._2=Ply_object->Triangles[i].Vertices[2];

    Triangles_normals[i]=Ply_object->Triangles[i].Normal;
  }

  // random colors
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

  // create drawarrays
  // vertices
  Vertices_drawarray.resize(Triangles.size()*3);
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_drawarray[i*3]=Vertices[Triangles[i]._0];
    Vertices_drawarray[i*3+1]=Vertices[Triangles[i]._1];
    Vertices_drawarray[i*3+2]=Vertices[Triangles[i]._2];
  }
  // vertices colors
  Vertices_colors_drawarray.resize(Triangles.size()*3);
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_colors_drawarray[i*3]=Vertices_colors[Triangles[i]._0];
    Vertices_colors_drawarray[i*3+1]=Vertices_colors[Triangles[i]._1];
    Vertices_colors_drawarray[i*3+2]=Vertices_colors[Triangles[i]._2];
  }

  // vertices triangles colors
  Vertices_triangles_colors_drawarray.resize(Triangles.size()*3);
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_triangles_colors_drawarray[i*3]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+1]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+2]=Triangles_colors[i];
  }

  // vertices triangle normals
  Vertices_triangles_normals_drawarray.resize(Triangles.size()*3);
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_triangles_normals_drawarray[i*3]=Triangles_normals[i];
    Vertices_triangles_normals_drawarray[i*3+1]=Triangles_normals[i];
    Vertices_triangles_normals_drawarray[i*3+2]=Triangles_normals[i];
  }

  // vertices normals
  Vertices_normals_drawarray.resize(Triangles.size()*3);
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_normals_drawarray[i*3]=Vertices_normals[Triangles[i]._0];
    Vertices_normals_drawarray[i*3+1]=Vertices_normals[Triangles[i]._1];
    Vertices_normals_drawarray[i*3+2]=Vertices_normals[Triangles[i]._2];
  }

  Vertices_texture_coordinates_drawarray.resize(Triangles.size()*3);
  for (unsigned int i=0;i<Triangles.size();i++){
    Vertices_texture_coordinates_drawarray[i*3]=Vertices_texture_coordinates[Triangles[i]._0];
    Vertices_texture_coordinates_drawarray[i*3+1]=Vertices_texture_coordinates[Triangles[i]._1];
    Vertices_texture_coordinates_drawarray[i*3+2]=Vertices_texture_coordinates[Triangles[i]._2];
  }

  Initial_position=0;
  Num_vertices=Vertices_drawarray.size();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _texture_object::draw()
{
  glDrawArrays(GL_TRIANGLES,Initial_position,Num_vertices);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _texture_object::create_data_drawarrays()
{
  // for each triangle
  Vertices_drawarray.resize(Triangles.size()*3);
  Vertices_colors_drawarray.resize(Triangles.size()*3);
  Vertices_triangles_colors_drawarray.resize(Triangles.size()*3);
  Vertices_normals_drawarray.resize(Triangles.size()*3);
  Vertices_triangles_normals_drawarray.resize(Triangles.size()*3);
  Vertices_texture_coordinates_drawarray.resize(Triangles.size()*3);

  for (unsigned int i=0;i<Triangles.size();i++){
    // fill the vertices
    Vertices_drawarray[i*3]=Vertices[Triangles[i]._0];
    Vertices_drawarray[i*3+1]=Vertices[Triangles[i]._1];
    Vertices_drawarray[i*3+2]=Vertices[Triangles[i]._2];
    // fill the verices colors
    Vertices_colors_drawarray[i*3]=Vertices_colors[Triangles[i]._0];
    Vertices_colors_drawarray[i*3+1]=Vertices_colors[Triangles[i]._1];
    Vertices_colors_drawarray[i*3+2]=Vertices_colors[Triangles[i]._2];
    // fill the verices colors
    Vertices_triangles_colors_drawarray[i*3]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+1]=Triangles_colors[i];
    Vertices_triangles_colors_drawarray[i*3+2]=Triangles_colors[i];
    // fill the normals
    Vertices_normals_drawarray[i*3]=Vertices_normals[Triangles[i]._0];
    Vertices_normals_drawarray[i*3+1]=Vertices_normals[Triangles[i]._1];
    Vertices_normals_drawarray[i*3+2]=Vertices_normals[Triangles[i]._2];
    // fill the normals of triangles
    Vertices_triangles_normals_drawarray[i*3]=Triangles_normals[i];
    Vertices_triangles_normals_drawarray[i*3+1]=Triangles_normals[i];
    Vertices_triangles_normals_drawarray[i*3+2]=Triangles_normals[i];
    // fill the texture coordinates
    Vertices_texture_coordinates_drawarray[i*3]=Vertices_texture_coordinates[Triangles[i]._0];
    Vertices_texture_coordinates_drawarray[i*3+1]=Vertices_texture_coordinates[Triangles[i]._1];
    Vertices_texture_coordinates_drawarray[i*3+2]=Vertices_texture_coordinates[Triangles[i]._2];
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _texture_object::update_texture_data_drawarrays(_parametrizer *Parametrizer1)
{
  list<_parametrizer_ns::_point>::iterator It_p;
  int Model_triangle;

  for(unsigned int Patch = 0; Patch < Parametrizer1->Vector_patches.size(); Patch++){
//  for(unsigned int Patch = 0; Patch < 1; Patch++){
    for(unsigned int Triangle = 0; Triangle < Parametrizer1->Vector_patches[Patch]->Vector_triangles.size();Triangle++){
      Model_triangle=Parametrizer1->Vector_patches[Patch]->Vector_triangles[Triangle].Model_triangle;
      for (int i=0;i<3;i++){
        It_p=Parametrizer1->Vector_patches[Patch]->Vector_triangles[Triangle].It_points[i];
        Vertices_drawarray[Model_triangle*3+i]=(*It_p).Texture_coordinates;
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
