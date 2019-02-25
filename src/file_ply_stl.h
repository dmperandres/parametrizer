/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef _READ_PLY
#define _READ_PLY

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include "vertex.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _file_ply
{
public:

  _file_ply(){}
  int open(const string &File_name, string Mode);
  void read(vector<_vertex3f> &Vertices, vector<_vertex3i> &Triangles);
  void write(vector<_vertex3f> &Vertices, vector<_vertex3i> &Triangles, vector<_vertex2f> &Normals);

  void close();

private:
  void get_token(std::istringstream &Line_stream1,std::string &Token1);
  bool get_new_line(std::fstream &File,std::istringstream &Line_stream);

  std::fstream File;
};
#endif
