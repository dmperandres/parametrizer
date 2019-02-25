/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <GL/glew.h>
#include <QMainWindow>
#include <QMenuBar>
#include <QGLFormat>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QApplication>
#include <QScreen>
#include <vector>

#include "vertex.h"

class _gl_widget;

namespace _window_ne {
  const float SCREEN_WIDTH_FACTOR=0.5f;
  const float SCREEN_HEIGHT_FACTOR=0.9f;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _window : public QMainWindow
{
Q_OBJECT

public:
  _window();
  void save_data(QString File_name,std::vector<std::vector<int>> &Results);
  void save_data_time(QString File_name, std::vector<std::vector<float>> &Times);
  void save_data_error(QString File_name, std::vector<std::vector<_vertex3f>> &Errors);

public slots:
  void load_ply_slot();
  void save_ply_slot();
//  void run_test_slot();

private:
  _gl_widget *GL_widget;

  QActionGroup *Action_group;
  std::vector<QAction *> Vec_actions;
};

#endif
