/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "file_ply_stl.h"

using namespace std;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

int _file_ply::open(const string &File_name,string Mode)
{

  if (Mode=="r") File.open(File_name.c_str(),ios::in);
  else{
    if (Mode=="w") File.open(File_name.c_str(),ios::out);
  }

  if (File.good()) return 1;
  else {
    cout << "Error: the file " << File_name << " cannot be opened" << endl;
    return 0;
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _file_ply::get_token(istringstream &Line_stream1,string &Token1)
{
//  cout <<  "buscando en:" << Line_stream1.str() << endl;
  getline(Line_stream1,Token1,' ');
//  cout <<  "salida:" << Token1 << endl;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool _file_ply::get_new_line(fstream &File,istringstream &Line_stream)
{
  string Line;
  if (getline(File,Line)){
//    cout << "leido:" << Line << endl;
    Line_stream.str(Line);
    Line_stream.seekg(0);
    return true;
  }
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _file_ply::read(vector<_vertex3f> &Vertices,vector<_vertex3i> &Triangles)
{
  string Token;
  istringstream Line_stream;

  setlocale(LC_ALL, "C");

  get_new_line(File,Line_stream);
  get_token(Line_stream,Token);
  if(Token=="ply"){
    get_new_line(File,Line_stream);
    get_token(Line_stream,Token);
    if(Token=="format"){
      get_token(Line_stream,Token);
      if(Token=="ascii"){
        do{
          get_new_line(File,Line_stream);
          get_token(Line_stream,Token);
        } while (Token!="element");
        get_token(Line_stream,Token);
        if(Token=="vertex"){
          get_token(Line_stream,Token);
          Vertices.resize(stoi(Token));
          do{
            get_new_line(File,Line_stream);
            get_token(Line_stream,Token);
          } while (Token!="element");
          get_token(Line_stream,Token);
          if(Token=="face"){
            get_token(Line_stream,Token);
            Triangles.resize(stoi(Token));
            do{
              get_new_line(File,Line_stream);
              get_token(Line_stream,Token);
            } while (Token!="end_header");
            for (unsigned int i=0;i<Vertices.size();i++){
              get_new_line(File,Line_stream);
              //
              get_token(Line_stream,Token);
              Vertices[i].x=stof(Token);
              get_token(Line_stream,Token);
              Vertices[i].y=stof(Token);
              get_token(Line_stream,Token);
              Vertices[i].z=stof(Token);

//              cout << " leido " << Vertices[i].x << " " << Vertices[i].y << " " << Vertices[i].z << endl;
            }
            for (unsigned int i=0;i<Triangles.size();i++){
              get_new_line(File,Line_stream);
              //
              get_token(Line_stream,Token);
              if (stoi(Token)==3){
                get_token(Line_stream,Token);
                Triangles[i]._0=stoi(Token);
                get_token(Line_stream,Token);
                Triangles[i]._1=stoi(Token);
                get_token(Line_stream,Token);
                Triangles[i]._2=stoi(Token);
              }
            }
          }
          else{
            cerr << "Error: no face element found" << endl;
          }
        }
        else{
          cerr << "Error: no vertex element found" << endl;
        }
      }
      else{
        cerr << "Error: only ascii format is allowed" << endl;
      }
    }
    else{
      cerr << "Error: no format found" << endl;
    }
  }
  else{
    cerr << "Error: the file is not a PLY file" << endl;
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _file_ply::write(vector<_vertex3f> &Vertices,vector<_vertex3i> &Triangles,vector<_vertex2f> &Normals)
{

  File << "ply" << endl;
  File << "format ascii 1.0" << endl;
  File << "element vertex " << Vertices.size() << endl;
  File << "property float32 x" << endl;
  File << "property float32 y" << endl;
  File << "property float32 z" << endl;
  File << "property float32 s" << endl;
  File << "property float32 t" << endl;
  File << "element face "<< Triangles.size() << endl;
  File << "property list uint8 int32 vertex_indices" << endl;
  File << "end_header" << endl;
  for (unsigned int i=0;i<Vertices.size();i++){
    File << std::to_string(Vertices[i].x) << " " << std::to_string(Vertices[i].y) << " " << std::to_string(Vertices[i].z) << " " << std::to_string(Normals[i].s) << " " << std::to_string(Normals[i].t) << endl;
  }
  for (unsigned int i=0;i<Triangles.size();i++){
    File << "3 " << std::to_string(Triangles[i].x) << " " << std::to_string(Triangles[i].y) << " " << std::to_string(Triangles[i].z) << endl;
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _file_ply::close()
{
  File.close();
}
