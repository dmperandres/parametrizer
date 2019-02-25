/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "accuracy.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

float _accuracy_ns::accuracy(float Value)
{
  Value*=_accuracy_ns::ACCURACY_INT;
  Value=round(Value);
  Value/=_accuracy_ns::ACCURACY_INT;

  return Value;
}
