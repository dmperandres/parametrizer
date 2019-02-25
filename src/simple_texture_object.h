/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef SIMPLE_TEXTURE_OBJECT_H
#define SIMPLE_TEXTURE_OBJECT_H

#include <iostream>
#include "basic_object3d.h"

class _object3D;
class _parameterizer;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _simple_texture_object: public _basic_object3d
{
public:

  _simple_texture_object();
  void draw();
};


#endif
