/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <GL/glew.h>
#include <QOpenGLWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QImageReader>
#include <QGuiApplication>
#include <QDir>
#include <QFrame>
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include <QRadioButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QString>
#include <QDoubleSpinBox>

#include <iostream>
#include <fstream>
#include <random>
#include <memory>

#include "shaders.h"
#include <vertex.h>

#include "axis.h"
#include "file_ply_stl.h"
#include "texture_object.h"
#include "simple_texture_object.h"
#include "button_color_simple.h"
#include "material.h"

#include "parametrizador.h"
#include "object3D.h"

#include "table_info.h"

namespace _gl_widget_ne {

  enum {VISIBILITY,WRITABILITY};

  typedef enum {CLASS_TYPE_ENTITY,CLASS_TYPE_VALUE} _class_type;
  typedef enum {DATA_INPUT_ENUMERATED,DATA_INPUT_INTERPOLATED} _data_input;
  typedef enum {DATA_TYPE_TEXT,DATA_TYPE_INTEGER,DATA_TYPE_FLOAT,DATA_TYPE_PHOTOGRAPH,DATA_TYPE_VIDEO,DATA_TYPE_URL,DATA_TYPE_AUDIO} _data_type;
  typedef enum {COLOR_MODEL_RGB,COLOR_MODEL_HSV} _color_model;

  const vector<QString> Vec_data_types={{"Text"},{"Integer"},{"Float"},{"Photograph"},{"Video"},{"URL"},{"Audio"}};

  const vector<_class_type> Vec_class_type={CLASS_TYPE_ENTITY,CLASS_TYPE_VALUE,CLASS_TYPE_VALUE,CLASS_TYPE_ENTITY,CLASS_TYPE_ENTITY,CLASS_TYPE_ENTITY,CLASS_TYPE_ENTITY};

  const vector<QString> Vec_headers_enumerated_palette={
    {"Color;Text"},
    {"Color;Value"},
    {"Color;Value"},
    {"Color;Text;Resource"},
    {"Color;Text;Resource"},
    {"Color;Text;Resource"},
    {"Color;Text;Resource"}
  };

  const vector<int> Vec_num_columns_headers_enumerated_palette={2,2,2,3,3,3,3};
  const vector<vector<int>> Vec_width_columns_headers_enumerated_palette={
    {50},
    {50},
    {50},
    {50,120},
    {50,120},
    {50,120},
    {50,120},
    {50,120}
  };

  typedef enum {MODE_POINT,MODE_LINE,MODE_FILL} _mode_drawing;
  typedef enum {MODE_SOLID, MODE_INTERPOLATE_COLORS} _mode_color;
  typedef enum {MODE_FLAT_SHADING, MODE_PHONG_SHADING} _mode_interpolation;
  typedef enum {PERSPECTIVE_PROJECTION,PARALLEL_PROJECTION} _projection_type;

  const int DEFAULT_COLOR=0;

  const float X_MIN=-1;
  const float X_MAX=1;
  const float Y_MIN=-1;
  const float Y_MAX=1;
  const float FRONT_PLANE_PARALLEL=-5000;
  const float BACK_PLANE_PARALLEL=5000;
  const float FRONT_PLANE_PERSPECTIVE=(X_MAX-X_MIN);
  const float BACK_PLANE_PERSPECTIVE=1000;

  const float MAX_AXIS_SIZE=5000;
  const float DEFAULT_DISTANCE=(X_MAX-X_MIN)*2;
  const float ANGLE_STEP_KEY=1;
  const float ANGLE_STEP_MOUSE=0.1f;

  const float DISTANCE_STEP=0.01f;
  const float DISTANCE_FACTOR=0.9f;

  const float DEFAULT_SCALE_FACTOR=10;
  const float SCALE_STEP=DEFAULT_SCALE_FACTOR/10;

  const float DEFAULT_TEXTURE_SCALE_FACTOR=2;
  const float DEFAULT_TEXTURE_TRANSLATION_FACTOR=DEFAULT_TEXTURE_SCALE_FACTOR/2;

  const int DEFAULT_WINDOW_WIDTH=512;
  const int DEFAULT_WINDOW_HEIGHT=512;
  const unsigned char DEFAULT_TONE=240;

  const int COLOR_POINT=0;
  const int COLOR_LINE=1;
  const int COLOR_FILL=8;

  const float MAX_LENGTH_AXIS=5000;
  typedef enum{ACTION_SELECTION,ACTION_CHANGE_CAMERA,ACTION_SELECTION_LINES_VP,ACTION_NOTHING,ACTION_LAST} _action_type;

  const unsigned int MAX_LAYERS=256;
  const unsigned int MAX_COLORS=255;

  const int MIN_ENUMERATED_VALUE=0;
  const int MAX_ENUMERATED_VALUE=MAX_COLORS-1;
  const int VOID_ENUMERATED_VALUE=MAX_COLORS;

  const int MIN_INTEGER_VALUE=0;
  const int MAX_INTEGER_VALUE=1000;
  const int VOID_INTEGER_VALUE=-1;

  const float MIN_FLOAT_VALUE=0.0;
  const float MAX_FLOAT_VALUE=1.0;
  const float VOID_FLOAT_VALUE=-1.0;
}

class _light {
public:
  _vertex4f Position;
  _vertex3f Color;
};

class _texture{
public:
  GLuint ID;
  uint Width;
  uint Height;
};

class _camera_data{
public:
  float Distance;
  _vertex2f Traslation;
};

class _window;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

class _gl_widget : public QOpenGLWidget
{
    Q_OBJECT
public:
  _gl_widget(_window *Window1);
  ~_gl_widget() override;

  void initialize_axis_data(vector<_vertex3f> &Axis_vertices, vector<_vertex3f> &Axis_colors);
  void initialize_axis();

  void initialize_object3D();
  void release_object3D();
  void update_object_ply();

  void initialize_texture_object();
  void release_texture_object();
  void update_texture_object();

  void initialize_simple_texture_object();
  void release_simple_texture_object();

  void load_model(const QString &File_name);
  void save_model(const QString &File_name);

  void load_image(const QString &File_name, QImage &Image1);

  void load_shaders();

  void draw_objects();
  void draw_texture();

  QLayout *gl_main_layout(){return Horizontal_layout;}

  // control
  void transparence(float Value);

  _vertex3f color_point(){return Color_point;}
  _vertex3f color_line(){return Color_line;}
  _vertex3f color_fill(){return Color_fill;}

  bool draw_point(){return Draw_point;}
  bool draw_line(){return Draw_line;}
  bool draw_fill(){return Draw_fill;}

  bool lighting(){return Lighting;}

  GLuint set_image(QImage &Image1);

  void update_info();

  void pick();

  void run_parametrization(){Parametrizer.run_parametrization();}
  void order_method(int Method1){Parametrizer.set_order_criterion(Method1);}
  void distortion(float Distortion1){Parametrizer.set_distorsion(Distortion1);}

  void parametrization_patch_max(float Value1){Parametrizer.set_patch_max(Value1);}
  int parametrization_num_patches(){return int(Parametrizer.Vector_patches.size());}
  float parametrization_time_parametrization(){return Parametrizer.time_parametrization();}
  float parametrization_total_error(){return Parametrizer.total_error();}
  float parametrization_mean_error(){return Parametrizer.mean_error();}
  float parametrization_max_error(){return Parametrizer.max_error();}

public slots:
  void order_method_slot(int Method1);
  void distortion_slot(double Distortion1);
  void patch_max_slot(double Value1);

  void run_parameterization_slot();

  void activate_point_slot(int State);
  void activate_line_slot(int State);
  void activate_fill_slot(int State);

  void activate_lighting_slot(int State);
  void activate_texturing_slot(int State);

  void activate_shading_slot(bool State);

  void activate_mode_color_slot(bool State);

  void color_background_slot(QColor Color);
  void color_point_slot(QColor Color);
  void color_line_slot(QColor Color);
  void color_fill_slot(QColor Color);

protected:
  void resizeGL(int Width1, int Height1) Q_DECL_OVERRIDE;
  void paintGL() Q_DECL_OVERRIDE;
  void initializeGL() Q_DECL_OVERRIDE;
  void keyPressEvent(QKeyEvent *Keyevent) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *Event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *Event) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent *Event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *Event) Q_DECL_OVERRIDE;
  void wheelEvent(QWheelEvent *Event) Q_DECL_OVERRIDE;


  // create widgets
  void create_parameterization_widget();
  void create_view_widget();

  void read_ply_file(const std::string & filename);

private:
  _window *Window;

  // data for the axis
  GLuint Program_axis;
  GLuint VAO_axis;
  GLuint VBO_vertices_axis;
  GLuint VBO_colors_axis;

  // data for the ply object
  GLuint Program_draw;
  GLuint VAO1;
  GLuint VBO_vertices1;
  GLuint VBO_colors1;
  GLuint VBO_triangles_normals1;
  GLuint VBO_vertices_normals1;
  GLuint VBO_texture_coordinates1;
  GLuint VBO_triangles_colors1;

  vector<_texture> Vec_textures;

  // data for the parametrized version of ply object
  GLuint Program_pick;
  GLuint VAO2;
  GLuint VBO_vertices2;
  GLuint VBO_colors2;
  GLuint VBO_triangles_normals2;
  GLuint VBO_vertices_normals2;
  GLuint VBO_texture_coordinates2;
  GLuint VBO_triangles_colors2;

  // data for drawing the square that represents the border of the texture
  GLuint VAO3;
  GLuint VBO_vertices3;
  GLuint VBO_colors3;
  GLuint VBO_triangles_normals3;
  GLuint VBO_vertices_normals3;
  GLuint VBO_texture_coordinates3;
  GLuint VBO_triangles_colors3;

  //
  float Back_plane_perspective;
  _camera_data Camera_data[2];

  float Distance_step;

  float Angle_camera_x;
  float Angle_camera_y;
  float Distance;

  float Angle_light_x;
  float Angle_light_y;

  int Window_width;
  int Window_height;

  int Axis_vertices_size;
  int Sphere_vertices_size;

  bool Draw_axis[2];

  bool Draw_point;
  bool Draw_line;
  bool Draw_fill;

  _light Light;
  _vertex3f Ambient_coeff;
  _gl_widget_ne::_projection_type Projection_type;
  float Scale_factor;
  float Mix_value;
  bool Change_camera;

  int Initial_position_x;
  int Initial_position_y;
  bool Change_position;

  _material Default_material;

  // Axis
  unique_ptr<_axis> Axis;

  // texture_object
  unique_ptr<_texture_object> Texture_object;
  unique_ptr<_simple_texture_object> Simple_texture_object;

  // widgets
  QTabWidget *Tab_widget;
  QHBoxLayout *Horizontal_layout;

  QWidget *Widget_layers;
  QWidget *Widget_view;

  QSlider *Slider_layer_transparence;
  QPushButton *Button_run_parameterization;

  QRadioButton *Radiobutton_shading_flat;
  QRadioButton *Radiobutton_mode_color[3];

  _table_info *Table_info;

  //
  _vertex3f Color_background;
  _vertex3f Color_point;
  _vertex3f Color_line;
  _vertex3f Color_fill;

  bool Lighting;
  bool Texturing;

  bool Shading_flat;

  int Mode_color;
  //
  _parametrizer Parametrizer;
  shared_ptr<_object3D> Object3D;

  bool Ply_loaded;
  bool Texture_loaded;

  uint Mode_visualization;

  bool Selection;
  bool Show_selection;
  int Selection_position_x;
  int Selection_position_y;

  GLuint FBO;
  GLuint Color_texture;
  GLuint Depth_texture;

  int Selected_triangle;
  int Selected_triangle1;
  int Selected_triangle2;
  int Selected_triangle3;
};
#endif
