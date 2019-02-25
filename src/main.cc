/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>
#include <fstream>
#include <iostream>
#include "window.h"

#define REDIRECT


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

int main( int argc, char ** argv )
{
  QApplication Application( argc, argv );
  _window Window;

  QSurfaceFormat Format;
  Format.setDepthBufferSize(24);
  Format.setStencilBufferSize(8);
  QSurfaceFormat::setDefaultFormat(Format);

  QFont Font("Arial");
  Font.setPointSize(10);
  Application.setFont(Font);

  Window.show();
  return Application.exec();
}
