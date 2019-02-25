/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef _BUTTON_COLOR_SIMPLE_H
#define _BUTTON_COLOR_SIMPLE_H

#include <QPushButton>
#include <QColor>
#include <QColorDialog>
#include <iostream>

using namespace std;

class _gl_widget;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _button_color_simple: public QPushButton
{
Q_OBJECT

public:
  _button_color_simple(QColor Color1);

public:signals:
  void click_button(QColor);

protected:
  void mouseDoubleClickEvent(QMouseEvent *Event);

  QColor Color;
};

#endif
