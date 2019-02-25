/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "button_color_simple.h"
#include "glwidget.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_button_color_simple::_button_color_simple(QColor Color1)
{
  Color=Color1;

  QString qss = QString("background-color: %1").arg(Color1.name());
  setStyleSheet(qss);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _button_color_simple::mouseDoubleClickEvent(QMouseEvent *Event)
{
  Q_UNUSED(Event)
  Color = QColorDialog::getColor(Color,nullptr, "Select Color");

  if(Color.isValid()) {
    QString qss = QString("background-color: %1").arg(Color.name());//+QString(";border: 0px;");
    setStyleSheet(qss);

    emit click_button(Color);
  }
}
