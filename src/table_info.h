/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef _TABLE_INFO_H
#define _TABLE_INFO_H

#include <QTableWidget>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include <QSize>
#include <QHeaderView>
#include <QDropEvent>
#include <iostream>
#include <vector>


using namespace std;

namespace _table_info_ne {
  const int NUM_COLS=2;
  const int NUM_ROWS=8;
}

class _gl_widget;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _table_info: public QTableWidget
{
Q_OBJECT

public:
  _gl_widget *GL_widget;

  _table_info(_gl_widget *GL_widget1);

  void set_valuei(int Row,int Value);
  void set_valuef(int Row,float Value);

protected:
  // for update
  void remove_all_rows();

  // for interchanging
  void set_row(int row, const QList<QTableWidgetItem*>& rowItems);
};
#endif
