/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef RECTANGLE_PACKER_H
#define RECTANGLE_PACKER_H

#include <vector>
#include <list>
#include "vertex.h"
#include "accuracy.h"

namespace _rectangle_packer_ns {


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  struct _free_rectangle
  {
    float Area=0;
    float Ratio=0;
    _vertex2f Position;
    _vertex2f Size;
    bool Bottom_right;
    int Level;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  struct _rectangle
  {
    float Area=0;
    _vertex2f Position;
    _vertex2f Size;
    float Ratio;
    float Inv_ratio;
    bool Rotate=false;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _order_rectangle{
  public:
    float Area=-1;
    int Rectangle_index=-1;
  };


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

  class _rectangles_packer
  {
  public:
    _rectangles_packer(){}
    float pack(vector<_rectangle> &Vector_rectangles);
    void add_free_rectangle(float x, float y, float Width1, float Height1, int Level1);
    bool pack_rectangles(float Width1, vector<_rectangle> &Vector_rectangles, list<_order_rectangle> &List_order_rectangles1);
    bool try_case(float Size1, vector<_rectangle> &Vector_rectangles, list<_order_rectangle> &List_order_rectangles);
    float border(){return Border;}

    list<_free_rectangle> List_free_rectangles;

    float Area_rectangles=0;
    float Min_area;
    float Min_side;
    float Border;
    int Max_level=0;
    float Total_area;
    float Ocuppied_area;
    float Ratio_area_ocuppied;
  };
}
#endif
