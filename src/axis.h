/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef AXIS_H
#define AXIS_H

#include "basic_object3d.h"

namespace _axis_ne {
  const _vertex3f COLORS[]={{0,0,0},{1,0,0},{0,1,0},{0,0,1},{0,1,1},{1,0,1},{1,1,0},{1,1,1}};
  const float MAX_AXIS_SIZE=10000;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _axis: public _basic_object3d
{
public:
  _axis();

  void draw();
};
#endif


