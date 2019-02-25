/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "table_info.h"
#include "glwidget.h"

using namespace _table_info_ne;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_table_info::_table_info(_gl_widget *GL_widget1)
{
  GL_widget=GL_widget1;

  setRowCount(NUM_ROWS);
  setColumnCount(NUM_COLS);

  verticalHeader()->hide();

  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  setHorizontalHeaderLabels(QString("Parameter;Value").split(";"));

  setColumnWidth(0,170);
  horizontalHeader()->setStretchLastSection(true);

  QTableWidgetItem *Item = new QTableWidgetItem("Vertices");
  setItem(0,0,Item);
  Item = new QTableWidgetItem("0");
  setItem(0,1,Item);

  Item = new QTableWidgetItem("Triangles");
  setItem(1,0,Item);
  Item = new QTableWidgetItem("0");
  setItem(1,1,Item);

  Item = new QTableWidgetItem("Patches");
  setItem(2,0,Item);
  Item = new QTableWidgetItem("0");
  setItem(2,1,Item);

  Item = new QTableWidgetItem("Total error %");
  setItem(3,0,Item);
  Item = new QTableWidgetItem("0");
  setItem(3,1,Item);

  Item = new QTableWidgetItem("Mean error %");
  setItem(4,0,Item);
  Item = new QTableWidgetItem("0");
  setItem(4,1,Item);

  Item = new QTableWidgetItem("Max error %");
  setItem(5,0,Item);
  Item = new QTableWidgetItem("0");
  setItem(5,1,Item);

  Item = new QTableWidgetItem("Param. time (s)");
  setItem(6,0,Item);
  Item = new QTableWidgetItem("0.00");
  setItem(6,1,Item);

  Item = new QTableWidgetItem("Pack. time (s)");
  setItem(7,0,Item);
  Item = new QTableWidgetItem("0.00");
  setItem(7,1,Item);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _table_info::set_valuei(int Row,int Value)
{
  QTableWidgetItem *Item=item(Row,1);
  Item->setText(QString("%1").arg(Value));
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _table_info::set_valuef(int Row,float Value)
{
  QTableWidgetItem *Item=item(Row,1);
  Item->setText(QString().setNum(Value,'f',6));
}
