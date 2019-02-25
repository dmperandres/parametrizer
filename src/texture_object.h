/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef TEXTURE_OBJECT_H
#define TEXTURE_OBJECT_H

#include <iostream>

#include "basic_object3d.h"

class _object3D;
class _parametrizer;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _texture_object: public _basic_object3d
{
public:

  _texture_object();
  void create(_object3D *Ply_object);
  void create_data_drawarrays();
  void update_texture_data_drawarrays(_parametrizer *Parametrizer1);
  void draw();
};
#endif
