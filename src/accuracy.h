/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef ACCURACY_H
#define ACCURACY_H

#include <math.h>


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

namespace _accuracy_ns {

  const int ACCURACY=6;
  const int ACCURACY_INT=int(powf(10,ACCURACY));

  float accuracy(float Value);
}
#endif
