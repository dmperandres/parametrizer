/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "window.h"
#include "glwidget.h"


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_window::_window()
{
  QSizePolicy Q(QSizePolicy::Expanding,QSizePolicy::Expanding);

  QWidget *Central_widget = new QWidget(this);

  QGLFormat Format;
  Format.setAlpha(true);
  Format.setAccum(true);
  Format.setRgba(true);
  Format.setDoubleBuffer(true);
  QGLFormat::setDefaultFormat(Format);

  GL_widget = new _gl_widget(this);
  GL_widget->setSizePolicy(Q);
  GL_widget->setMinimumSize(300,300);

  Central_widget->setLayout(GL_widget->gl_main_layout());
  setCentralWidget(Central_widget);

  // actions for file menu
  QAction *Load_ply = new QAction(QIcon("./icons/fileopen.png"), tr("&Load ply model"), this);
  Load_ply->setToolTip(tr("Load a ply file"));
  connect(Load_ply, SIGNAL(triggered()), this, SLOT(load_ply_slot()));

  QAction *Save_ply = new QAction(QIcon("./icons/filesave.png"), tr("&Save ply model"), this);
  Save_ply->setToolTip(tr("Save the current ply model"));
  connect(Save_ply, SIGNAL(triggered()), this, SLOT(save_ply_slot()));

//  QAction *Run_test = new QAction(QIcon("./icons/filesave.png"), tr("&Run test"), this);
//  Run_test->setToolTip(tr("Run test"));
//  connect(Run_test, SIGNAL(triggered()), this, SLOT(run_test_slot()));

  QAction *Exit = new QAction(QIcon("./icons/exit.png"), tr("&Exit..."), this);
  Exit->setShortcut(tr("Ctrl+Q"));
  Exit->setToolTip(tr("Exit the application"));
  connect(Exit, SIGNAL(triggered()), this, SLOT(close()));

  // menus
  QMenu *File_menu=menuBar()->addMenu(tr("&File"));
  File_menu->addAction(Load_ply);
  File_menu->addAction(Save_ply);
//  File_menu->addSeparator();
//  File_menu->addAction(Run_test);
  File_menu->addSeparator();
  File_menu->addAction(Exit);
  File_menu->setAttribute(Qt::WA_AlwaysShowToolTips);

  QRect Screen_size = QApplication::primaryScreen()->geometry();
  resize(Screen_size.width()*_window_ne::SCREEN_WIDTH_FACTOR,Screen_size.height()*_window_ne::SCREEN_HEIGHT_FACTOR);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _window::load_ply_slot()
{
QString File_name1 = QFileDialog::getOpenFileName(this, tr("Open File"),"./ply",tr("Images (*.ply)"));

if (!File_name1.isEmpty()){
  QFile File(File_name1);
  if (!File.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::warning(this, tr("Application"),tr("Cannot read file %1:\n%2.").arg(File_name1).arg(File.errorString()));
    return;
    }
  GL_widget->load_model(File_name1);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

//void _window::run_test_slot()

//{
//  QString File_name1={"ply/"};
//  QString File_name2={"results/"};
//  vector<QString> Files={"urn2","beethoven","big_dodge","bunny","vasija"};
//  vector<int> Order_metod={0,1,2,3};
//  vector<QString> Order_method_text={"_order0","_order1","_order2","_order3"};
//  vector<float> Distortion={0.0,1.0,5.0,10.0,25.0};
//  vector<QString> Distortion_text={"_dist00","_dist01","_dist05","_dist10","_dist25"};
//  vector<float> Patch_max={100.0,50.0,10.0,1.0};
//  vector<QString> Patch_max_text={"_patch100","_patch050","_patch010","_patch001"};
//  vector<vector<int>> Results;
//  vector<vector<float>> Times;
//  vector<vector<_vertex3f>> Errors;

//  Results.resize(Distortion.size());
//  Times.resize(Distortion.size());
//  Errors.resize(Distortion.size());
//  for (unsigned int i=0;i<Distortion.size();i++){
//    Results[i].resize(Patch_max.size());
//    Times[i].resize(Patch_max.size());
//    Errors[i].resize(Patch_max.size());
//  }

//  for (unsigned int Counter=0;Counter<Files.size();Counter++){
//    GL_widget->load_model(File_name1+Files[Counter]+".ply");
//    for (unsigned int Counter1=0;Counter1<Order_metod.size();Counter1++){
//      GL_widget->order_method(Order_metod[Counter1]);
//      for (unsigned int Counter2=0;Counter2<Distortion.size();Counter2++){
//        GL_widget->distortion(Distortion[Counter2]);
//        for (unsigned int Counter3=0;Counter3<Patch_max.size();Counter3++){
//          GL_widget->parametrization_patch_max(Patch_max[Counter3]);
//          GL_widget->run_parametrization();
//          Results[Counter2][Counter3]=GL_widget->parametrization_num_patches();
//          Times[Counter2][Counter3]=GL_widget->parametrization_time_parametrization();
//          Errors[Counter2][Counter3]._0=GL_widget->parametrization_total_error();
//          Errors[Counter2][Counter3]._1=GL_widget->parametrization_mean_error();
//          Errors[Counter2][Counter3]._2=GL_widget->parametrization_max_error();
//        }
//      }
//      save_data(File_name2+Files[Counter]+Order_method_text[Counter1]+".txt",Results);
//      save_data_time(File_name2+Files[Counter]+Order_method_text[Counter1]+"_time.txt",Times);
//      save_data_error(File_name2+Files[Counter]+Order_method_text[Counter1]+"_error.txt",Errors);
//    }
//  }
//}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _window::save_data(QString File_name, vector<vector<int>> &Results)
{
  std::ofstream File;

  cout << "Saving file " << File_name.toStdString() << endl;

  File.open(File_name.toStdString(),std::ofstream::out);
  for (unsigned int i=0;i<Results.size();i++){
    for (unsigned int j=0;j<Results[i].size();j++){
      File << std::to_string(Results[i][j]) << "; ";
    }
    File << std::endl;
  }
  File.close();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _window::save_data_time(QString File_name,vector<vector<float>> &Times)
{
  std::ofstream File;

  cout << "Saving file " << File_name.toStdString() << endl;

  File.open(File_name.toStdString(),std::ofstream::out);
  for (unsigned int i=0;i<Times.size();i++){
    for (unsigned int j=0;j<Times[i].size();j++){
      File << std::to_string(Times[i][j]) << "; ";
    }
    File << std::endl;
  }
  File.close();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _window::save_data_error(QString File_name,vector<vector<_vertex3f>> &Errors)
{
  std::ofstream File;

  cout << "Saving file " << File_name.toStdString() << endl;

  File.open(File_name.toStdString(),std::ofstream::out);
  for (unsigned int i=0;i<Errors.size();i++){
    for (unsigned int j=0;j<Errors[i].size();j++){
      File << std::to_string(Errors[i][j]._0) << "; " << std::to_string(Errors[i][j]._1) << "; "<< std::to_string(Errors[i][j]._2) << "; " ;
    }
    File << std::endl;
  }
  File.close();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _window::save_ply_slot()
{
  QString File_name1 = QFileDialog::getSaveFileName(this, tr("Save ply"),"./ply",tr("Images (*.ply)"));

  if (!File_name1.isEmpty()){
    GL_widget->save_model(File_name1);
  }
}
