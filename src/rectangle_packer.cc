/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "rectangle_packer.h"
#include <algorithm>
#include "parametrizador.h"

using namespace std;
using namespace _rectangle_packer_ns;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool compare_patches(const _order_rectangle& First, const _order_rectangle& Second)
{
  // order: first greater values
  if (First.Area>Second.Area) return true;
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _rectangles_packer::add_free_rectangle(float x, float y, float Width1, float Height1, int Level1)
{
  x=_accuracy_ns::accuracy(x);
  y=_accuracy_ns::accuracy(y);
  Width1=_accuracy_ns::accuracy(Width1);
  Height1=_accuracy_ns::accuracy(Height1);

  float Area=Width1*Height1;

  // the space is useless
  if (Area<Min_area || Width1<Min_side || Height1<Min_side) return;

  _free_rectangle Free_rectangle;
  list<_free_rectangle>::iterator It_lfr;

  Free_rectangle.Area=Area;
  Free_rectangle.Size.x=Width1;
  Free_rectangle.Size.y=Height1;
  Free_rectangle.Position.x=x;
  Free_rectangle.Position.y=y;
  Free_rectangle.Level=Level1;

  It_lfr=List_free_rectangles.begin();

  while (It_lfr!=List_free_rectangles.end() && (*It_lfr).Area>Area) It_lfr++;

  List_free_rectangles.insert(It_lfr,Free_rectangle);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool _rectangles_packer::pack_rectangles(float Width1,vector<_rectangle> &Vector_rectangles, list<_order_rectangle> &List_order_rectangles1)
{
  list<_free_rectangle>::iterator It_lfr;
  _free_rectangle Free_rectangle;
  _order_rectangle Order_rectangle;
  int Rectangle_index;
  bool Valid;
  bool Continue;
  int Orientation=0;
  float Area;
  float Width;
  float Height;
  float Diff_x;
  float Diff_y;
  float Ratio1;
  float Ratio2;

  list<_order_rectangle> List_order_rectangles(List_order_rectangles1);

  // create the free space
  List_free_rectangles.clear();
  add_free_rectangle(0,0,Width1,Width1,1);
  _vertex2f Position_aux;
  _vertex2f Size_aux;

  Total_area=Width1*Width1;
  Ocuppied_area=0;

  // while  there is rectangles to include
  Continue=true;
  while (List_order_rectangles.size()>0 && Continue==true){
    // get form the ordered list
    Order_rectangle=List_order_rectangles.front();

    Rectangle_index=Order_rectangle.Rectangle_index;
    // remove element
    List_order_rectangles.pop_front();
    // get the area
    Area=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Area;
    // search in the list of free_rectangles until the position where the area is smaller than the area of the rectangle to include

    It_lfr=List_free_rectangles.begin();
    while (next(It_lfr)!=List_free_rectangles.end() && (*It_lfr).Area>=Area) It_lfr++;
    // from this position backward search for the best free_rectangle
    Valid=false;
    while (It_lfr!=List_free_rectangles.end()){
      // check if the rectangle can be included in the free rectangle
      // in the original orientation
      Orientation=0;
      Size_aux=(*It_lfr).Size;
      if (Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.x<=(*It_lfr).Size.x && Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.y<=(*It_lfr).Size.y){
        Valid=true;
        break;
      }
      // in the rotated orientation
      if (Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.x<=(*It_lfr).Size.y && Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.y<=(*It_lfr).Size.x){
        Valid=true;
        Orientation=1;
        break;
      }
      It_lfr--;
    }

    if (Valid==true){
      // it is possible to insert the rectangle

      // compute the new position for the included rectangle
      Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Position=(*It_lfr).Position;
      Ocuppied_area+=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Area;

      if (Orientation==0){
        // rotated 90 degrees
        Width=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.x;
        Height=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.y;
        Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Rotate=false;
      }
      else{
//        if (Change_value==true) Vector_rectangles[Rectangle_index].Rotate=true;
        Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Rotate=true;

        Width=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.y;
        Height=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.x;
      }

      // remove the free_rectangle used
      Free_rectangle=(*It_lfr);
      List_free_rectangles.erase(It_lfr);

      if (Free_rectangle.Level>Max_level) Max_level=Free_rectangle.Level;

      // add the new two zones in the ordered list of free_rectangles
      // first check the wich zone has a better ratio (near to 1)
      Diff_x=_accuracy_ns::accuracy(Free_rectangle.Size.x-Width);
      Diff_y=_accuracy_ns::accuracy(Free_rectangle.Size.y-Height);

      if (Diff_x==0 || Diff_y==0){
        if (Diff_x==0 && Diff_y!=0){
          add_free_rectangle(Free_rectangle.Position.x,Free_rectangle.Position.y+Height,Width,Diff_y,Free_rectangle.Level+1);
        }
        else{
          if (Diff_x!=0){
            add_free_rectangle(Free_rectangle.Position.x+Width,Free_rectangle.Position.y,Diff_x,Height,Free_rectangle.Level+1);
          }
        }
      }
      else{
        if (Free_rectangle.Size.y>Diff_x) Ratio1=Free_rectangle.Size.y/Diff_x;
        else Ratio1=Diff_x/Free_rectangle.Size.y;

        if (Free_rectangle.Size.x>Diff_y) Ratio2=Free_rectangle.Size.x/Diff_y;
        else Ratio2=Diff_y/Free_rectangle.Size.x;

        if (Ratio1<=Ratio2){
          // the big new free rectangle is at right-bottom side
          add_free_rectangle(Free_rectangle.Position.x+Width,Free_rectangle.Position.y,Diff_x,Free_rectangle.Size.y,Free_rectangle.Level+1);
          add_free_rectangle(Free_rectangle.Position.x,Free_rectangle.Position.y+Height,Width,Diff_y,Free_rectangle.Level+1);
        }
        else{
          // the big new free rectangle is at top-left side
          add_free_rectangle(Free_rectangle.Position.x,Free_rectangle.Position.y+Height,Free_rectangle.Size.x,Diff_y,Free_rectangle.Level+1);
          add_free_rectangle(Free_rectangle.Position.x+Width,Free_rectangle.Position.y,Diff_x,Height,Free_rectangle.Level+1);
        }
      }
    }
    else{
      // not possible to introduce the rectangle
      Continue=false;
    }
  }

//  if (Continue==true){
//    cout << "Possible to pack with size=" << Width1 << endl;
//  }
//  else{
//    cout << "Impossible to pack with size=" << Width1 << endl;
//  }

  Ratio_area_ocuppied=Ocuppied_area/Total_area;

  return Continue;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

bool _rectangles_packer::try_case(float Size1,vector<_rectangle> &Vector_rectangles, list<_order_rectangle> &List_order_rectangles)
{
  if (float(pow(Size1,2))>Area_rectangles){
    // get value
    if (pack_rectangles(Size1,Vector_rectangles,List_order_rectangles)) return true;
    else return false;
  }
  else return false;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

float _rectangles_packer::pack(vector<_rectangle> &Vector_rectangles)
{
  list<_order_rectangle> List_order_rectangles;
  list<_order_rectangle>::iterator It_lor;
  _order_rectangle Order_rectangle;

  float Area;
  float x,y;
  Min_area=1e8;
  Min_side=1e8;

  for (unsigned int Rectangle=0;Rectangle<Vector_rectangles.size();Rectangle++){
    Area=Vector_rectangles[Rectangle].Area;
    if (Min_area>Area) Min_area=Area;

    Area_rectangles+=Area;
    Order_rectangle.Area=Area;
    Order_rectangle.Rectangle_index=int(Rectangle);

    x=Vector_rectangles[Rectangle].Size.x;
    y=Vector_rectangles[Rectangle].Size.y;

    if (Min_side>x) Min_side=x;
    if (Min_side>y) Min_side=y;

    List_order_rectangles.push_back(Order_rectangle);
  }

  Min_area*=0.9f;
  Min_side*=0.9f;

  // order the zones by area: first the bigger ones
  List_order_rectangles.sort(compare_patches);

  // first rectangle
  It_lor=List_order_rectangles.begin();
  int Rectangle_index; //=(*It_lor).Rectangle_index;
  float Side1=0;
  float Side2=0;
  float Side3=0;
  float Final_size=0;
  float Min_size=0;
  float Max_size=0;

  // we are goint to find two points: the max dimension that does not fit and the min dimension that does fit

  bool Not_found=true;
  while (It_lor!=List_order_rectangles.end() && Not_found){
    // get the two dimensions of the next bigger rectangle    
    Rectangle_index=(*It_lor).Rectangle_index;
    Side2=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.x;
    Side3=Vector_rectangles[static_cast<unsigned long>(Rectangle_index)].Size.y;

    // first try to pack the rectangles with the smaller side
    if (Side2!=Side3){
      if (Side2<Side3){
        // try side 2 first
        if (try_case(Side1+Side2,Vector_rectangles,List_order_rectangles)){
          Not_found=false;
          Max_size=Side1+Side2;
        }
        else{
          // try Side3 second
          if (try_case(Side1+Side3,Vector_rectangles,List_order_rectangles)){
            Not_found=false;
            Max_size=Side1+Side3;
          }
          else{
            Side1=Side1+Side3;
            Min_size=Side1;
            It_lor++;
          }
        }
      }
      else{
        // first Side3
        if (try_case(Side1+Side3,Vector_rectangles,List_order_rectangles)){
          Not_found=false;
          Max_size=Side1+Side3;
        }
        else{
          // second side2
          if (try_case(Side1+Side2,Vector_rectangles,List_order_rectangles)){
            Not_found=false;
            Max_size=Side1+Side2;
          }
          else{
            Side1=Side1+Side2;
            Min_size=Side1;
            It_lor++;
          }
        }
      }
    }
    else{ // the sides are equal
      if (try_case(Side1+Side2,Vector_rectangles,List_order_rectangles)){
        Not_found=false;
        Max_size=Side1+Side2;
      }
      else{
        Side1=Side1+Side2;
        Min_size=Side1;
        It_lor++;
      }
    }
  }

  if (Max_level>1){
    float Ratio_area_prev=0;
    float Ratio_area_next=Ratio_area_ocuppied;

    // search
    while (Ratio_area_next>Ratio_area_prev && abs(Ratio_area_next-Ratio_area_prev)>0.1f){
      // try with the mean distance
      if (try_case((Min_size+Max_size)/2,Vector_rectangles,List_order_rectangles)){
        Max_size=(Min_size+Max_size)/2;
        Ratio_area_next=Ratio_area_ocuppied;
      }
      else{
        Min_size=(Min_size+Max_size)/2;
        Ratio_area_prev=(Ratio_area_prev+Ratio_area_next)/2;
      }
    }
  }

  Final_size=Max_size;

//  cout << "Max_level=" << Max_level << endl;

  if (Max_level%2==1) Max_level++;

  // this part adds the border to each patch
  Border=Final_size/_parametrizer_ns::DEFAULT_DIVISOR;

  // add to the rectangles the border
  vector<_rectangle> Vector_rectangles1(Vector_rectangles);

  Final_size=Final_size+Border*2*Max_level;

  for (unsigned int i=0;i<Vector_rectangles1.size();i++){
    Vector_rectangles[i].Size.x+=Border*2;
    Vector_rectangles[i].Size.y+=Border*2;
  }

  Not_found=true;
  do{
    if (try_case(Final_size,Vector_rectangles,List_order_rectangles)){
      Not_found=false;
//      cout << "Ok" << endl;
    }
    else{
      Final_size*=1.1f;
    }
  } while (Not_found);

  return Final_size;
}
