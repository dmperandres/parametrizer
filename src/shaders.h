/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef SHADERS_H
#define SHADERS_H

#include <string>
#include <GL/glew.h>
#include <iostream>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>

using namespace std;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _shaders
{
public:
  GLuint load_shaders(QString File_vertex_shader, QString File_fragment_shader);

protected:
  bool  read_file(QString File_name,QString &Code);
};

#endif
