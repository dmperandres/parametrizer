//LIC

#ifndef MATERIAL_H
#define MATERIAL_H

#include "vertex.h"

//HEA

class _material
{
public:
  _vertex3f Constant_color;
  _vertex3f Ambient_color;
  _vertex3f Diffuse_color;
  _vertex3f Specular_color;
  float Specular_exponent;
};
#endif


