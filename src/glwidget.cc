/*! \file
 * Copyright Domingo Martín Perandres
 * email: dmartin@ugr.es
 * web: http://calipso.ugr.es/dmartin
 * 2019
 * GPL
 */


#include "glwidget.h"

using namespace std;
using namespace _gl_widget_ne;


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_gl_widget::_gl_widget(_window *Window1):Window(Window1)
{
  setMinimumSize(300, 300);
  setFocusPolicy(Qt::StrongFocus);

  Draw_point=false;
  Draw_line=false;
  Draw_fill=true;

  Angle_camera_x=0;
  Angle_camera_y=0;
  Distance=DEFAULT_DISTANCE;

  Distance_step=DISTANCE_STEP;

  Scale_factor=DEFAULT_SCALE_FACTOR;

  Light.Color=_vertex3f(1,1,1);

  Ambient_coeff=_vertex3f(0.2f,0.2f,0.2f);

  Lighting=false;
  Texturing=true;

  Shading_flat=true;

  Mode_color=1;

  Change_camera=true;

  Angle_light_x=0;
  Angle_light_y=0;

  Projection_type=PERSPECTIVE_PROJECTION;

//  Default_material={{0.5f,0.5f,0.5f},{1.0f,1.0f,1.0f},{.5f,.5f,.5f},{0.3f,0.3f,0.3f},10};

  Color_background=_vertex3f(1,1,1);
  Color_point=_vertex3f(0,0,0);
  Color_line=_vertex3f(1,0,0);
  Color_fill=_vertex3f(0.5,0.5,0.5);

  // widgets
  Horizontal_layout= new QHBoxLayout;
  Horizontal_layout->setContentsMargins(1,1,1,1);

  create_parameterization_widget();
  create_view_widget();

  Tab_widget=new QTabWidget;

  Tab_widget->addTab(Widget_layers, tr("Parameterization"));
  Tab_widget->addTab(Widget_view, tr("View"));
  Tab_widget->setMaximumWidth(300);

  Horizontal_layout->addWidget(this);
  Horizontal_layout->addWidget(Tab_widget);

  Ply_loaded=false;
  Texture_loaded=false;

  Vec_textures.resize(8);

  Axis=nullptr;
  Object3D=nullptr;
  Texture_object=nullptr;
  Simple_texture_object=nullptr;

  for (uint i=0;i<2;i++){
    Camera_data[i].Distance=X_MAX;
    Draw_axis[i]=true;
  }

  Mode_visualization=0;

  Selection=false;
  Show_selection=false;
  Selection_position_x=0;
  Selection_position_y=0;

  Selected_triangle=-1;
  Selected_triangle1=-1;
  Selected_triangle2=-1;
  Selected_triangle3=-1;

  Back_plane_perspective=BACK_PLANE_PERSPECTIVE;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

_gl_widget::~_gl_widget()
{
  Axis.reset();
  Object3D.reset();
  Texture_object.reset();
  Simple_texture_object.reset();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::load_model(const QString &File_name)
{
  vector<_vertex3f> Vertices;
  vector<_vertex3i> Triangles;
  _file_ply File;

  if (Object3D!=nullptr){
    release_object3D();
    release_texture_object();
    release_simple_texture_object();

    Object3D.reset();
    Texture_object.reset();
    Simple_texture_object.reset();

    Parametrizer.clear_data();
  }

  Object3D=make_shared<_object3D>();

  File.open(static_cast<const char *>(File_name.toStdString().c_str()),"r");
  File.read(Vertices,Triangles);
  File.close();

//  Object3D->open(static_cast<const char *>(File_name.toStdString().c_str()),"r");
//  Object3D->read_ply(coordenadas,indices);
//  Object3D->close();

  Object3D->create(Vertices,Triangles);
  Object3D->create_data_drawarrays();
  Object3D->compute_spherical_texture_coordinates();

  // adjust the camera
  float Max_dimension=Object3D->max_dimension();
  Distance=Max_dimension/0.8f; // 2=distance to front plane. 0.8 position of the projection (see dimension of the window)
  Back_plane_perspective=Max_dimension*2;

  for (uint i=0;i<2;i++){
    Camera_data[i].Distance=1.0f;
  }

  initialize_object3D();

  Parametrizer.set_ply_model(Object3D.get());

  Ply_loaded=true;

//  if (Texture_loaded)
  Button_run_parameterization->setEnabled(true);

  update_info();

  // texture object
  Texture_object=make_unique<_texture_object>();
  Texture_object->create(Object3D.get());
  Texture_object->create_data_drawarrays();
  initialize_texture_object();

  //
  Simple_texture_object=make_unique<_simple_texture_object>();
  initialize_simple_texture_object();

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::save_model(const QString &File_name)
{
   _file_ply File;
   vector<_vertex3f> Vertices;
   vector<_vertex3i> Triangles;
   vector<_vertex2f> Normals;

   Vertices.resize(Object3D->Vertices.size());
   Triangles.resize(Object3D->Triangles.size());
   Normals.resize(Object3D->Vertices.size());

   for (unsigned int i=0;i<Vertices.size();i++){
     Vertices[i]=Object3D->Vertices[i].Position;
     Normals[i]=Object3D->Vertices_texture_coordinates[i];
   }

   for (unsigned int i=0;Triangles.size();i++){
     Triangles[i]=Object3D->Triangles[i].Vertices;
   }

  File.open(static_cast<const char *>(File_name.toStdString().c_str()),"w");
  File.write(Vertices,Triangles,Normals);
  File.close();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::load_image(const QString &File_name,QImage &Image1)
{
  QImageReader Reader(File_name);
  Reader.setAutoTransform(true);
  Image1 = Reader.read();
  if (Image1.isNull()) {
      QMessageBox::information(this, QGuiApplication::applicationDisplayName(),tr("Cannot load %1.").arg(QDir::toNativeSeparators(File_name)));
      return;
  }
  Image1=Image1.mirrored();
  Image1=Image1.convertToFormat(QImage::Format_RGB888);

//  TR.leer(File_name.toStdString().c_str());
//  Texture_loaded=true;
//  if (Ply_loaded) Button_run_parameterization->setEnabled(true);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

GLuint _gl_widget::set_image(QImage &Image1)
{
  GLuint Texture;

  glCreateTextures(GL_TEXTURE_2D,1,&Texture);
  glTextureStorage2D(Texture,1,GL_RGB8,Image1.width(),Image1.height());

  glBindTexture(GL_TEXTURE_2D,Texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // fill with data
  glTextureSubImage2D(Texture,0,0,0,Image1.width(),Image1.height(),GL_RGB,GL_UNSIGNED_BYTE,Image1.constBits());

  return Texture;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::keyPressEvent(QKeyEvent *Keyevent)
{
  switch(Keyevent->key()){
  case Qt::Key_P:Draw_point=!Draw_point;break;
  case Qt::Key_L:Draw_line=!Draw_line;break;
  case Qt::Key_F:Draw_fill=!Draw_fill;break;

  case Qt::Key_I:Lighting=!Lighting;break;
  case Qt::Key_T:Texturing=!Texturing;break;

  case Qt::Key_B:Mode_visualization=!Mode_visualization;break;

  case Qt::Key_X:Draw_axis[Mode_visualization]=!Draw_axis[Mode_visualization];break;

  case Qt::Key_S:Show_selection=!Show_selection;break;

  case Qt::Key_1:Mode_color=0;break;
  case Qt::Key_2:Mode_color=1;break;
  case Qt::Key_3:Mode_color=2;break;

  case Qt::Key_Left:
    if (Change_camera==true) Angle_camera_y-=ANGLE_STEP_KEY;
    else Angle_light_y+=ANGLE_STEP_KEY;
    break;
  case Qt::Key_Right:
    if (Change_camera==true) Angle_camera_y+=ANGLE_STEP_KEY;
    else Angle_light_y-=ANGLE_STEP_KEY;
    break;
  case Qt::Key_Up:
    if (Change_camera==true) Angle_camera_x-=ANGLE_STEP_KEY;
    else Angle_light_x+=ANGLE_STEP_KEY;
    break;
  case Qt::Key_Down:
    if (Change_camera==true) Angle_camera_x+=ANGLE_STEP_KEY;
    else Angle_light_x-=ANGLE_STEP_KEY;
    break;
  case Qt::Key_PageUp:
    if (Projection_type==PERSPECTIVE_PROJECTION) Distance*=DISTANCE_FACTOR;
    else Scale_factor*=DISTANCE_FACTOR;
    break;
  case Qt::Key_PageDown:
    if (Projection_type==PERSPECTIVE_PROJECTION) Distance/=DISTANCE_FACTOR;
    else Scale_factor/=DISTANCE_FACTOR;
    break;

  case Qt::Key_M:
    if (Projection_type==PERSPECTIVE_PROJECTION) Projection_type=PARALLEL_PROJECTION;
    else Projection_type=PERSPECTIVE_PROJECTION;
    break;

  case Qt::Key_Plus:
    Mix_value+=0.1f;
    if (Mix_value>1) Mix_value=1.0f;
    break;
  case Qt::Key_Minus:
    Mix_value-=0.1f;
    if (Mix_value<0) Mix_value=0.0f;
    break;

  case Qt::Key_F1:Change_camera=!Change_camera;break;
  default:break;
  }
  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::mousePressEvent(QMouseEvent *Event)
{
  Initial_position_x=Event->x();
  Initial_position_y=Event->y();

  switch (Event->button()){
  case Qt::LeftButton:
    Change_position=true;
    break;
  case Qt::RightButton:
    Change_position=true;
    break;
  default:break;
  }

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::mouseReleaseEvent(QMouseEvent *Event)
{
  switch (Event->button()){
  case Qt::LeftButton:
    Change_position=false;
    if (Event->modifiers() & Qt::ControlModifier){
      Selection_position_x=Event->x();
      Selection_position_y=Window_height-Event->y();
      pick();
    }
    break;
  case Qt::RightButton:
    Change_position=false;
//    if (Event->button() & Qt::RightButton){
//        pick();
//        update();
//    }
    break;
  default:break;
  }
  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::mouseDoubleClickEvent(QMouseEvent *Event)
{
  switch (Event->button()){
  case Qt::LeftButton:
    if (Change_camera==true){
      Angle_camera_x=0;
      Angle_camera_y=0;
      Camera_data[Mode_visualization].Distance=1.0f;
      Distance_step=DISTANCE_STEP;
    }
    else{
      Angle_light_x=0;
      Angle_light_y=0;
    }
    break;
  case Qt::RightButton:
//    Distance=DEFAULT_DISTANCE;
    Camera_data[Mode_visualization].Traslation=_vertex2f(0,0);
    break;
  default:break;
  }

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::mouseMoveEvent(QMouseEvent *Event)
{
  int Last_position_x=Event->x();
  int Last_position_y=Event->y();

//  cout << Initial_position_x << " " << Last_position_x << endl;

  if (Event->buttons() & Qt::LeftButton){
    if (Change_camera==true){
      Angle_camera_y+=float(Last_position_x-Initial_position_x)*ANGLE_STEP_MOUSE;
      Angle_camera_x+=float(Last_position_y-Initial_position_y)*ANGLE_STEP_MOUSE;
    }
    else{
      Angle_light_y-=float(Last_position_x-Initial_position_x)*ANGLE_STEP_MOUSE;
      Angle_light_x-=float(Last_position_y-Initial_position_y)*ANGLE_STEP_MOUSE;
    }
    Initial_position_x=Last_position_x;
    Initial_position_y=Last_position_y;
  }
  if (Event->buttons() & Qt::RightButton){
    if (Change_position==true){
      Camera_data[Mode_visualization].Traslation.x+=float(Last_position_x-Initial_position_x)*Distance_step;
      Camera_data[Mode_visualization].Traslation.y+=float(Initial_position_y-Last_position_y)*Distance_step;
      Initial_position_x=Last_position_x;
      Initial_position_y=Last_position_y;
    }
  }
  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::wheelEvent(QWheelEvent *Event)
{
  int Step=Event->delta()/120;

  if (Step<0){
    Camera_data[Mode_visualization].Distance/=DISTANCE_FACTOR;
    Distance_step/=0.8f;
  }
  else{
    Camera_data[Mode_visualization].Distance*=DISTANCE_FACTOR;
    Distance_step*=0.8f;
  }

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::draw_objects()
{
  QMatrix4x4 Projection;
  QMatrix4x4 View;
  QMatrix4x4 Model;
  QMatrix4x4 Camera_matrix;
  QMatrix4x4 Light_matrix;
  QMatrix4x4 Normal_matrix;
  float Distance1=Camera_data[Mode_visualization].Distance;

  float Aspect=float(Window_height)/float(Window_width);

  if (Projection_type==PERSPECTIVE_PROJECTION){
//    Projection.frustum(-Distance1,Distance1,-Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PERSPECTIVE,BACK_PLANE_PERSPECTIVE);
    Projection.frustum(-Distance1,Distance1,-Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PERSPECTIVE,Back_plane_perspective);
  }
  else{
    Projection.ortho(-Distance1,Distance1,Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PARALLEL,BACK_PLANE_PARALLEL);
  }

  View.translate(0,0,-Distance);
  View.rotate(Angle_camera_x,1,0,0);
  View.rotate(Angle_camera_y,0,1,0);
  View.translate(Camera_data[Mode_visualization].Traslation.x,Camera_data[Mode_visualization].Traslation.y);

  // camera position
  Camera_matrix.rotate(-Angle_camera_y,0,1,0);
  Camera_matrix.rotate(-Angle_camera_x,1,0,0);
  QVector3D Camera_position=QVector3D((Camera_matrix*QVector4D(0,0,Distance,1)));
  // light position
  Light_matrix.rotate(-Angle_light_y,0,1,0);
  Light_matrix.rotate(-Angle_light_x,1,0,0);
  QVector4D Light_position=Light_matrix*QVector4D(0,0,1,0);

  glClearColor(Color_background.r,Color_background.g,Color_background.b,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // axis
  if (Draw_axis[Mode_visualization]==true){
    glUseProgram(Program_axis);
    glBindVertexArray(VAO_axis);

    glUniformMatrix4fv(10,1,GL_FALSE,Model.data());
    glUniformMatrix4fv(11,1,GL_FALSE,View.data());
    glUniformMatrix4fv(12,1,GL_FALSE,Projection.data());

    Axis->draw();

    glBindVertexArray(0);
    glUseProgram(0);
  }

  // object
  if (Object3D!=nullptr){
    glUseProgram(Program_draw);
    glBindVertexArray(VAO1);

    glUniformMatrix4fv(10,1,GL_FALSE,Model.data());
    glUniformMatrix4fv(11,1,GL_FALSE,View.data());
    glUniformMatrix4fv(12,1,GL_FALSE,Projection.data());
    glUniformMatrix4fv(13,1,GL_FALSE,Normal_matrix.data());

    // default material
//    glUniform3fv(27,1,(GLfloat*) &Default_material.Constant_color); // fill color
    glUniform3fv(27,1,static_cast<GLfloat *>(&Default_material.Constant_color.x)); // fill color


    if (Draw_point){
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glPointSize(3);
      glUniform1i(20,MODE_POINT);
      glUniform1i(21,int(MODE_SOLID));// interpolation
      glUniform1i(22,int(false)); // no illumination
      glUniform3fv(25,1,static_cast<GLfloat *>(&Color_point.x));

      Object3D->draw();
    }

    if (Draw_line){
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glPolygonOffset(-1,1);
      glUniform1i(20,MODE_LINE);
      glUniform1i(21,int(MODE_SOLID));// interpolation
      glUniform1i(22,int(false)); // no illumination
      glUniform3fv(26,1,static_cast<GLfloat *>(&Color_line.x));

      Object3D->draw();
    }

    if (Draw_fill){
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glUniform1i(20,MODE_FILL);
      glUniform1i(21,Mode_color);
      glUniform1i(22,int(Lighting));
      glUniform1i(23,int(Shading_flat));
      glUniform1i(24,int(Texturing));

      glUniform3fv(27,1,static_cast<GLfloat*>(&Color_fill.x));
      glUniform3fv(15,1,static_cast<GLfloat *>(&Camera_position[0]));
      glUniform3fv(30,1,static_cast<GLfloat*>(&Light_position[0]));
      glUniform3fv(31,1,static_cast<GLfloat*>(&Light.Color.x));
      glUniform3fv(35,1,static_cast<GLfloat*>(&Ambient_coeff.x));

      if (Show_selection){
         if (Selected_triangle!=-1){
           glUniform1i(40,Selected_triangle);
           glUniform1i(41,Selected_triangle1);
           glUniform1i(42,Selected_triangle2);
           glUniform1i(43,Selected_triangle3);
         }
       }
       else{
         glUniform1i(40,-1);
         glUniform1i(41,-1);
         glUniform1i(42,-1);
         glUniform1i(43,-1);
       }

      // texture
      // sphere
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,Vec_textures[0].ID);

      Object3D->draw();
    }

    glUseProgram(0);
    glBindVertexArray(0);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::draw_texture()
{
  QMatrix4x4 Projection;
  QMatrix4x4 View;
  QMatrix4x4 Model;
  QMatrix4x4 Camera_matrix;
  QMatrix4x4 Light_matrix;
  QMatrix4x4 Normal_matrix;
  float Distance1=Camera_data[Mode_visualization].Distance;

  float Aspect=float(Window_height)/float(Window_width);

  Projection.ortho(-Distance1,Distance1,-Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PARALLEL,BACK_PLANE_PARALLEL);

  View.translate(0,0,-Distance);
  View.rotate(0,1,0,0);
  View.rotate(0,0,1,0);
  View.translate(Camera_data[Mode_visualization].Traslation.x,Camera_data[Mode_visualization].Traslation.y);

  // camera position
  Camera_matrix.rotate(0,0,1,0);
  Camera_matrix.rotate(0,1,0,0);
  QVector3D Camera_position=QVector3D((Camera_matrix*QVector4D(0,0,Distance,1)));
  // light position
  Light_matrix.rotate(-Angle_light_y,0,1,0);
  Light_matrix.rotate(-Angle_light_x,1,0,0);
  QVector4D Light_position=Light_matrix*QVector4D(0,0,1,0);

  glClearColor(Color_background.r,Color_background.g,Color_background.b,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // axis
  if (Draw_axis[Mode_visualization]==true){
    glUseProgram(Program_axis);
    glBindVertexArray(VAO_axis);

    glUniformMatrix4fv(10,1,GL_FALSE,Model.data());
    glUniformMatrix4fv(11,1,GL_FALSE,View.data());
    glUniformMatrix4fv(12,1,GL_FALSE,Projection.data());

    Axis->draw();

    glBindVertexArray(0);
    glUseProgram(0);
  }


  View.translate(-DEFAULT_TEXTURE_TRANSLATION_FACTOR,-DEFAULT_TEXTURE_TRANSLATION_FACTOR);
  View.scale(DEFAULT_TEXTURE_SCALE_FACTOR,DEFAULT_TEXTURE_SCALE_FACTOR,1.0);

  // draw the border of the 2D texture
  if (Simple_texture_object!=nullptr){
    glUseProgram(Program_draw);
    glBindVertexArray(VAO3);

    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glPolygonOffset(0,-1);

    glUniformMatrix4fv(10,1,GL_FALSE,Model.data());
    glUniformMatrix4fv(11,1,GL_FALSE,View.data());
    glUniformMatrix4fv(12,1,GL_FALSE,Projection.data());

    glUniform1i(20,MODE_LINE);
    glUniform1i(21,int(MODE_SOLID));// interpolation
    glUniform1i(22,int(false)); // no illumination
    glUniform3fv(26,1,static_cast<GLfloat*>(&Color_line.x));

    Simple_texture_object->draw();

    glBindVertexArray(0);
    glUseProgram(0);
  }

  // program to draw the textured ply object
  if (Texture_object!=nullptr){
    glUseProgram(Program_draw);
    glBindVertexArray(VAO2);

    glUniformMatrix4fv(10,1,GL_FALSE,Model.data());
    glUniformMatrix4fv(11,1,GL_FALSE,View.data());
    glUniformMatrix4fv(12,1,GL_FALSE,Projection.data());
    glUniformMatrix4fv(13,1,GL_FALSE,Normal_matrix.data());

    // default material
    glUniform3fv(27,1,static_cast<GLfloat*>(&Default_material.Constant_color.x));

    if (Draw_point){
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glPointSize(3);
      glUniform1i(20,MODE_POINT);
      glUniform1i(21,int(MODE_SOLID));// interpolation
      glUniform1i(22,int(false)); // no illumination
      glUniform3fv(25,1,static_cast<GLfloat*>(&Color_point.x));

      Texture_object->draw();
    }

    if (Draw_line){
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glPolygonOffset(0,-1);
      glUniform1i(20,MODE_LINE);
      glUniform1i(21,int(MODE_SOLID));// interpolation
      glUniform1i(22,int(false)); // no illumination
      glUniform3fv(26,1,static_cast<GLfloat*>(&Color_line.x));

      Texture_object->draw();
    }

    if (Draw_fill){
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glUniform1i(20,MODE_FILL);
      glUniform1i(21,Mode_color);
      glUniform1i(21,Mode_color);
      glUniform1i(22,int(Lighting));
      glUniform1i(23,int(Shading_flat));
      glUniform1i(24,int(Texturing));

      glUniform3fv(27,1,static_cast<GLfloat*>(&Color_fill.x));

      glUniform3fv(15,1,static_cast<GLfloat *>(&Camera_position[0]));
      glUniform3fv(30,1,static_cast<GLfloat*>(&Light_position[0]));
      glUniform3fv(31,1,static_cast<GLfloat*>(&Light.Color.x));
      glUniform3fv(35,1,static_cast<GLfloat*>(&Ambient_coeff.x));

      if (Show_selection){
         if (Selected_triangle!=-1){
           glUniform1i(40,Selected_triangle);
           glUniform1i(41,Selected_triangle1);
           glUniform1i(42,Selected_triangle2);
           glUniform1i(43,Selected_triangle3);
         }
       }
       else{
         glUniform1i(40,-1);
         glUniform1i(41,-1);
         glUniform1i(42,-1);
         glUniform1i(43,-1);
       }

      // texture
      // sphere
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,Vec_textures[0].ID);

      Texture_object->draw();
    }

    glUseProgram(0);
    glBindVertexArray(0);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::paintGL()
{
  if (Mode_visualization==0) draw_objects();
  else draw_texture();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::resizeGL(int Width1, int Height1)
{
  Window_width=Width1;
  Window_height=Height1;

  glViewport(0,0,Width1,Height1);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::load_shaders()
{
   _shaders Shader;

  Program_axis=Shader.load_shaders("shaders/axis.vert","shaders/axis.frag");
  if (Program_axis==0){
    exit(-1);
  }

  // program to draw the objects
  Program_draw=Shader.load_shaders("shaders/draw.vert","shaders/draw.frag");
  if (Program_draw==0){
    exit(-1);
  }

  // program to do the pick
  Program_pick=Shader.load_shaders("shaders/pick.vert","shaders/pick.frag");
  if (Program_pick==0){
    exit(-1);
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::initialize_axis()
{
  Axis=make_unique<_axis>();

  // axis
  glCreateVertexArrays(1,&VAO_axis);
  glBindVertexArray(VAO_axis);

  glCreateBuffers(1,&VBO_vertices_axis);
  glNamedBufferStorage(VBO_vertices_axis,6*3*sizeof(GLfloat),&Axis->Vertices_drawarray[0],GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO_axis,0,VBO_vertices_axis,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO_axis,0,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO_axis,0,0);
  glEnableVertexArrayAttrib(VAO_axis,0);

  glCreateBuffers(1,&VBO_colors_axis);
  glNamedBufferStorage(VBO_colors_axis,6*3*sizeof(GLfloat),&Axis->Vertices_colors_drawarray[0],GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO_axis,1,VBO_colors_axis,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO_axis,1,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO_axis,1,1);
  glEnableVertexArrayAttrib(VAO_axis,1);

  glBindVertexArray(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::initialize_object3D()
{
  // get the context
  makeCurrent();

  // object
  glCreateVertexArrays(1,&VAO1);
  glBindVertexArray(VAO1);

  glCreateBuffers(1,&VBO_vertices1);
  glNamedBufferStorage(VBO_vertices1,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(GLfloat)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO1,0,VBO_vertices1,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO1,0,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO1,0,0);
  glEnableVertexArrayAttrib(VAO1,0);

  glCreateBuffers(1,&VBO_colors1);
  glNamedBufferStorage(VBO_colors1,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(GLfloat)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO1,1,VBO_colors1,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO1,1,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO1,1,1);
  glEnableVertexArrayAttrib(VAO1,1);

  glCreateBuffers(1,&VBO_triangles_normals1);
  glNamedBufferStorage(VBO_triangles_normals1,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO1,2,VBO_triangles_normals1,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO1,2,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO1,2,2);
  glEnableVertexArrayAttrib(VAO1,2);

  glCreateBuffers(1,&VBO_vertices_normals1);
  glNamedBufferStorage(VBO_vertices_normals1,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO1,3,VBO_vertices_normals1,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO1,3,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO1,3,3);
  glEnableVertexArrayAttrib(VAO1,3);

  glCreateBuffers(1,&VBO_texture_coordinates1);
  glNamedBufferStorage(VBO_texture_coordinates1,static_cast<GLsizeiptr>(Object3D->Vertices_texture_coordinates_drawarray.size()*2*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO1,4,VBO_texture_coordinates1,0,2*sizeof(float));
  glVertexArrayAttribFormat(VAO1,4,2,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO1,4,4);
  glEnableVertexArrayAttrib(VAO1,4);

  glCreateBuffers(1,&VBO_triangles_colors1);
  glNamedBufferStorage(VBO_triangles_colors1,static_cast<GLsizeiptr>(Object3D->Vertices_triangles_colors_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO1,5,VBO_triangles_colors1,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO1,5,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO1,5,5);
  glEnableVertexArrayAttrib(VAO1,5);

  // Put data
  // vertices
  glNamedBufferSubData(VBO_vertices1,0,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Object3D->Vertices_drawarray[0]);
  // colors
  glNamedBufferSubData(VBO_colors1,0,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Object3D->Vertices_colors_drawarray[0]);
  // triangle normals
  glNamedBufferSubData(VBO_triangles_normals1,0,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Object3D->Vertices_triangles_normals_drawarray[0]);
  // vertices normals
  glNamedBufferSubData(VBO_vertices_normals1,0,static_cast<GLsizeiptr>(Object3D->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Object3D->Vertices_normals_drawarray[0]);
  // vertices texture coordinates
  glNamedBufferSubData(VBO_texture_coordinates1,0,static_cast<GLsizeiptr>(Object3D->Vertices_texture_coordinates_drawarray.size()*2*sizeof(GLfloat)),&Object3D->Vertices_texture_coordinates_drawarray[0]);
  // triangles colors
  glNamedBufferSubData(VBO_triangles_colors1,0,static_cast<GLsizeiptr>(Object3D->Vertices_triangles_colors_drawarray.size()*3*sizeof(GLfloat)),&Object3D->Vertices_triangles_colors_drawarray[0]);

  glBindVertexArray(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::release_object3D()
{
  makeCurrent();
  // object
  glDeleteVertexArrays(1,&VAO1);
  glDeleteBuffers(1,&VBO_vertices1);
  glDeleteBuffers(1,&VBO_colors1);
  glDeleteBuffers(1,&VBO_triangles_normals1);
  glDeleteBuffers(1,&VBO_vertices_normals1);
  glDeleteBuffers(1,&VBO_texture_coordinates1);
  glDeleteBuffers(1,&VBO_triangles_colors1);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::update_object_ply()
{
  // update drawarrays
  glBindVertexArray(VAO1);

  glNamedBufferSubData(VBO_texture_coordinates1,0,static_cast<GLsizeiptr>(Object3D->Vertices_texture_coordinates_drawarray.size()*2*sizeof(GLfloat)),&Object3D->Vertices_texture_coordinates_drawarray[0]);
  // triangles colors
  glNamedBufferSubData(VBO_triangles_colors1,0,static_cast<GLsizeiptr>(Object3D->Vertices_triangles_colors_drawarray.size()*3*sizeof(GLfloat)),&Object3D->Vertices_triangles_colors_drawarray[0]);

   glBindVertexArray(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::initialize_texture_object()
{
  // get the context
  makeCurrent();

  // object
  glCreateVertexArrays(1,&VAO2);
  glBindVertexArray(VAO2);

  glCreateBuffers(1,&VBO_vertices2);
  glNamedBufferStorage(VBO_vertices2,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO2,0,VBO_vertices2,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO2,0,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO2,0,0);
  glEnableVertexArrayAttrib(VAO2,0);

  glCreateBuffers(1,&VBO_colors2);
  glNamedBufferStorage(VBO_colors2,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO2,1,VBO_colors2,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO2,1,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO2,1,1);
  glEnableVertexArrayAttrib(VAO2,1);

  glCreateBuffers(1,&VBO_triangles_normals2);
  glNamedBufferStorage(VBO_triangles_normals2,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO2,2,VBO_triangles_normals2,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO2,2,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO2,2,2);
  glEnableVertexArrayAttrib(VAO2,2);

  glCreateBuffers(1,&VBO_vertices_normals2);
  glNamedBufferStorage(VBO_vertices_normals2,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO2,3,VBO_vertices_normals2,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO2,3,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO2,3,3);
  glEnableVertexArrayAttrib(VAO2,3);

  glCreateBuffers(1,&VBO_texture_coordinates2);
  glNamedBufferStorage(VBO_texture_coordinates2,static_cast<GLsizeiptr>(Texture_object->Vertices_texture_coordinates_drawarray.size()*2*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO2,4,VBO_texture_coordinates2,0,2*sizeof(float));
  glVertexArrayAttribFormat(VAO2,4,2,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO2,4,4);
  glEnableVertexArrayAttrib(VAO2,4);

  glCreateBuffers(1,&VBO_triangles_colors2);
  glNamedBufferStorage(VBO_triangles_colors2,static_cast<GLsizeiptr>(Texture_object->Vertices_triangles_colors_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO2,5,VBO_triangles_colors2,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO2,5,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO2,5,5);
  glEnableVertexArrayAttrib(VAO2,5);

  // Put data
  // vertices
  glNamedBufferSubData(VBO_vertices2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_drawarray[0]);
  // colors
  glNamedBufferSubData(VBO_colors2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_colors_drawarray[0]);
  // triangle normals
  glNamedBufferSubData(VBO_triangles_normals2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_triangles_normals_drawarray[0]);
  // vertices normals
  glNamedBufferSubData(VBO_vertices_normals2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_normals_drawarray[0]);
  // vertices texture coordinates
  glNamedBufferSubData(VBO_texture_coordinates2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_texture_coordinates_drawarray.size()*2*sizeof(GLfloat)),&Texture_object->Vertices_texture_coordinates_drawarray[0]);
  // triangles colors
  glNamedBufferSubData(VBO_triangles_colors2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_triangles_colors_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_triangles_colors_drawarray[0]);

  glBindVertexArray(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::release_texture_object()
{
  // get the context
  makeCurrent();
  glDeleteVertexArrays(1,&VAO2);
  glDeleteBuffers(1,&VBO_vertices2);
  glDeleteBuffers(1,&VBO_colors2);
  glDeleteBuffers(1,&VBO_triangles_normals2);
  glDeleteBuffers(1,&VBO_vertices_normals2);
  glDeleteBuffers(1,&VBO_texture_coordinates2);
  glDeleteBuffers(1,&VBO_triangles_colors2);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::update_texture_object()
{
  // update drawarrays
  glBindVertexArray(VAO2);

  // vertices
  glNamedBufferSubData(VBO_vertices2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_drawarray[0]);
  // colors
  glNamedBufferSubData(VBO_colors2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_colors_drawarray[0]);
  // texture coordinates
  glNamedBufferSubData(VBO_texture_coordinates2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_texture_coordinates_drawarray.size()*2*sizeof(GLfloat)),&Texture_object->Vertices_texture_coordinates_drawarray[0]);
  // triangles colors
  glNamedBufferSubData(VBO_triangles_colors2,0,static_cast<GLsizeiptr>(Texture_object->Vertices_triangles_colors_drawarray.size()*3*sizeof(GLfloat)),&Texture_object->Vertices_triangles_colors_drawarray[0]);

  glBindVertexArray(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::initialize_simple_texture_object()
{
  // get the context
  makeCurrent();

  // object
  glCreateVertexArrays(1,&VAO3);
  glBindVertexArray(VAO3);

  glCreateBuffers(1,&VBO_vertices3);
  glNamedBufferStorage(VBO_vertices3,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO3,0,VBO_vertices3,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO3,0,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO3,0,0);
  glEnableVertexArrayAttrib(VAO3,0);

  glCreateBuffers(1,&VBO_colors3);
  glNamedBufferStorage(VBO_colors3,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO3,1,VBO_colors3,0,3*sizeof(GLfloat));
  glVertexArrayAttribFormat(VAO3,1,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO3,1,1);
  glEnableVertexArrayAttrib(VAO3,1);

  glCreateBuffers(1,&VBO_triangles_normals3);
  glNamedBufferStorage(VBO_triangles_normals3,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO3,2,VBO_triangles_normals3,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO3,2,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO3,2,2);
  glEnableVertexArrayAttrib(VAO3,2);

  glCreateBuffers(1,&VBO_vertices_normals3);
  glNamedBufferStorage(VBO_vertices_normals3,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO3,3,VBO_vertices_normals3,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO3,3,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO3,3,3);
  glEnableVertexArrayAttrib(VAO3,3);

  glCreateBuffers(1,&VBO_texture_coordinates3);
  glNamedBufferStorage(VBO_texture_coordinates3,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_texture_coordinates_drawarray.size()*2*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO3,4,VBO_texture_coordinates3,0,2*sizeof(float));
  glVertexArrayAttribFormat(VAO3,4,2,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO3,4,4);
  glEnableVertexArrayAttrib(VAO3,4);

  glCreateBuffers(1,&VBO_triangles_colors3);
  glNamedBufferStorage(VBO_triangles_colors3,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_triangles_colors_drawarray.size()*3*sizeof(float)),nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  glVertexArrayVertexBuffer(VAO3,5,VBO_triangles_colors3,0,3*sizeof(float));
  glVertexArrayAttribFormat(VAO3,5,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayAttribBinding(VAO3,5,5);
  glEnableVertexArrayAttrib(VAO3,5);

  // Put data
  // vertices
  glNamedBufferSubData(VBO_vertices3,0,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Simple_texture_object->Vertices_drawarray[0]);
  // colors
  glNamedBufferSubData(VBO_colors3,0,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Simple_texture_object->Vertices_colors_drawarray[0]);
  // triangle normals
  glNamedBufferSubData(VBO_triangles_normals3,0,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Simple_texture_object->Vertices_triangles_normals_drawarray[0]);
  // vertices normals
  glNamedBufferSubData(VBO_vertices_normals3,0,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_drawarray.size()*3*sizeof(GLfloat)),&Simple_texture_object->Vertices_normals_drawarray[0]);
  // vertices texture coordinates
  glNamedBufferSubData(VBO_texture_coordinates3,0,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_texture_coordinates_drawarray.size()*2*sizeof(GLfloat)),&Simple_texture_object->Vertices_texture_coordinates_drawarray[0]);
  // triangles colors
  glNamedBufferSubData(VBO_triangles_colors3,0,static_cast<GLsizeiptr>(Simple_texture_object->Vertices_triangles_colors_drawarray.size()*3*sizeof(GLfloat)),&Simple_texture_object->Vertices_triangles_colors_drawarray[0]);

  glBindVertexArray(0);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::release_simple_texture_object()
{
  // get the context
  makeCurrent();

  // object
  glDeleteVertexArrays(1,&VAO3);
  glDeleteBuffers(1,&VBO_vertices3);
  glDeleteBuffers(1,&VBO_colors3);
  glDeleteBuffers(1,&VBO_triangles_normals3);
  glDeleteBuffers(1,&VBO_vertices_normals3);
  glDeleteBuffers(1,&VBO_texture_coordinates3);
  glDeleteBuffers(1,&VBO_triangles_colors3);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::initializeGL()
{
  const GLubyte* strm;

  strm = glGetString(GL_VENDOR);
  std::cerr << "Vendor: " << strm << "\n";
  strm = glGetString(GL_RENDERER);
  std::cerr << "Renderer: " << strm << "\n";
  strm = glGetString(GL_VERSION);
  std::cerr << "OpenGL Version: " << strm << "\n";

  if (strm[0] == '1'){
    std::cerr << "Only OpenGL 1.X supported!\n";
    exit(-1);
    }

  strm = glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cerr << "GLSL Version: " << strm << "\n";

  glewExperimental = GL_TRUE;
  int err = int(glewInit());
  if (GLEW_OK != err){
    std::cerr << "Error: " << glewGetErrorString(GLenum(err)) << "\n";
    exit (-1);
    }

  int Max_texture_size=0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &Max_texture_size);
  std::cout << "Max texture size: " << Max_texture_size << "\n";

  glClearColor(1.0,1.0,1.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POLYGON_OFFSET_LINE);

  Window_width=width();
  Window_height=height();

  load_shaders();
  initialize_axis();

  QImage Image;
  load_image("images/chessboard128.png",Image);
  Vec_textures[0].ID=set_image(Image);
  Vec_textures[0].Width=static_cast<unsigned int>(Image.width());
  Vec_textures[0].Height=static_cast<unsigned int>(Image.height());
  Texture_loaded=true;
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::run_parameterization_slot()
{
  if (Ply_loaded & Texture_loaded){
    Parametrizer.run_parametrization();
    Parametrizer.adjust_patches();

    Object3D->update_texture_data_drawarrays(&Parametrizer);
    update_object_ply();
    update_info();

    Texture_object->update_texture_data_drawarrays(&Parametrizer);
    update_texture_object();

    update();
  }
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::order_method_slot(int Method1)
{
  Parametrizer.set_order_criterion(Method1);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::distortion_slot(double Distortion1)
{
  Parametrizer.set_distorsion(float(Distortion1));
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::patch_max_slot(double Value1)
{
  Parametrizer.set_patch_max(float(Value1));
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_point_slot(int State)
{
  if (State==Qt::Checked) Draw_point=true;
  else Draw_point=false;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_line_slot(int State)
{
  if (State==Qt::Checked) Draw_line=true;
  else Draw_line=false;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_fill_slot(int State)
{
  if (State==Qt::Checked) Draw_fill=true;
  else Draw_fill=false;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_lighting_slot(int State)
{
  if (State==Qt::Checked) Lighting=true;
  else Lighting=false;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_texturing_slot(int State)
{
  if (State==Qt::Checked) Texturing=true;
  else Texturing=false;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_shading_slot(bool State)
{
  Q_UNUSED(State)
  if (Radiobutton_shading_flat->isChecked()==true) Shading_flat=true;
  else Shading_flat=false;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::activate_mode_color_slot(bool State)
{
  Q_UNUSED(State)
  for (uint i=0;i<3;i++){
    if (Radiobutton_mode_color[i]->isChecked()) Mode_color=int(i);
  }

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::color_background_slot(QColor Color)
{
  Color_background.r=float(Color.red())/255.0f;
  Color_background.g=float(Color.green())/255.0f;
  Color_background.b=float(Color.blue())/255.0f;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::color_point_slot(QColor Color)
{
  Color_point.r=float(Color.red())/255.0f;
  Color_point.g=float(Color.green())/255.0f;
  Color_point.b=float(Color.blue())/255.0f;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::color_line_slot(QColor Color)
{
  Color_line.r=float(Color.red())/255.0f;
  Color_line.g=float(Color.green())/255.0f;
  Color_line.b=float(Color.blue())/255.0f;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::color_fill_slot(QColor Color)
{
  Color_fill.r=float(Color.red())/255.0f;
  Color_fill.g=float(Color.green())/255.0f;
  Color_fill.b=float(Color.blue())/255.0f;

  update();
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::transparence(float Transparence1)
{
  Slider_layer_transparence->blockSignals(true);
  Slider_layer_transparence->setValue(int(Transparence1*255.0f));
  Slider_layer_transparence->blockSignals(false);
}


/*****************************************************************************//**
 * create widgets
 *
 *
 *
 *****************************************************************************/

void _gl_widget::create_parameterization_widget()
{
  Widget_layers=new QWidget;

  QVBoxLayout *Verticalbox_layers=new QVBoxLayout;

  // Palette
  QGroupBox *Groupbox_selection_method=new QGroupBox(tr("Selection method"));
  Groupbox_selection_method->setAlignment(Qt::AlignCenter);

  QVBoxLayout *Vertical_palette=new QVBoxLayout;


  QComboBox *Combobox_palette=new QComboBox;
  // update combobox
  Combobox_palette->addItem("Area descending");
  Combobox_palette->addItem("Area ascending");
  Combobox_palette->addItem("Angle descending");
  Combobox_palette->addItem("Angle ascending");

  connect(Combobox_palette,SIGNAL(currentIndexChanged(int)),this,SLOT(order_method_slot(int)));

  Vertical_palette->addWidget(Combobox_palette);
  Groupbox_selection_method->setLayout(Vertical_palette);

  // distortion spinbox
  QGroupBox *Groupbox_distortion=new QGroupBox(tr("Distortion"));
  Groupbox_distortion->setAlignment(Qt::AlignCenter);

  QVBoxLayout *Verticalbox_distortion=new QVBoxLayout;

  QDoubleSpinBox *Spinbox_distortion=new QDoubleSpinBox;
  Spinbox_distortion->setMinimum(0);
  Spinbox_distortion->setMaximum(100);
  Spinbox_distortion->setValue(double(Parametrizer.distortion()));

  Verticalbox_distortion->addWidget(Spinbox_distortion);
  Groupbox_distortion->setLayout(Verticalbox_distortion);
  connect(Spinbox_distortion, SIGNAL(valueChanged(double)),this,SLOT(distortion_slot(double)));

  // Patch max number of triangles
  QGroupBox *Groupbox_patch_max=new QGroupBox(tr("Max. Num. triangles/Patch"));
  Groupbox_patch_max->setAlignment(Qt::AlignCenter);

  QVBoxLayout *Verticalbox_patch_max=new QVBoxLayout;

  QDoubleSpinBox *Spinbox_patch_max=new QDoubleSpinBox;
  Spinbox_patch_max->setMinimum(0);
  Spinbox_patch_max->setMaximum(100);
  Spinbox_patch_max->setValue(double(Parametrizer.patch_max()));

  Verticalbox_patch_max->addWidget(Spinbox_patch_max);
  Groupbox_patch_max->setLayout(Verticalbox_patch_max);
  connect(Spinbox_patch_max, SIGNAL(valueChanged(double)),this,SLOT(patch_max_slot(double)));

  // table
  Table_info=new _table_info(this);

  // separator
  QFrame *Separator=new QFrame();
  Separator->setFrameStyle(QFrame::HLine);

  QWidget *Group_box_buttons=new QWidget;
  QHBoxLayout *Horizontal_box_buttons=new QHBoxLayout;

  Button_run_parameterization= new QPushButton("Run parameterization", this);
  Button_run_parameterization->setEnabled(false);

  Horizontal_box_buttons->addWidget(Button_run_parameterization);

  Group_box_buttons->setLayout(Horizontal_box_buttons);

  connect(Button_run_parameterization, SIGNAL(clicked()),this,SLOT(run_parameterization_slot()));

  Verticalbox_layers->addWidget(Groupbox_selection_method);
  Verticalbox_layers->addWidget(Groupbox_distortion);
  Verticalbox_layers->addWidget(Groupbox_patch_max);
  Verticalbox_layers->addWidget(Table_info);
  Verticalbox_layers->addWidget(Separator);
  Verticalbox_layers->addWidget(Group_box_buttons);
  Verticalbox_layers->addStretch();

  Widget_layers->setLayout(Verticalbox_layers);

  Widget_layers->setEnabled(true);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::create_view_widget()
{
  Widget_view=new QWidget;
  QVBoxLayout *Vertical_layout_view= new QVBoxLayout;

  // background
  QGroupBox *Groupbox_background=new QGroupBox(tr("Background"));
  Groupbox_background->setAlignment(Qt::AlignCenter);
  QGridLayout *Grid_layout_background = new QGridLayout;

  _button_color_simple *Button_background=new _button_color_simple(QColor(int(Color_background.r*255.0f),int(Color_background.g*255.0f),int(Color_background.b*255.0f)));

  Grid_layout_background->addWidget(Button_background,0,1,Qt::AlignCenter);

  Groupbox_background->setLayout(Grid_layout_background);

  connect(Button_background,SIGNAL(click_button(QColor)),this,SLOT(color_background_slot(QColor)));

  // point
  QGroupBox *Groupbox_point=new QGroupBox(tr("Point"));
  Groupbox_point->setAlignment(Qt::AlignCenter);
  QGridLayout *Grid_layout_point = new QGridLayout;
  QCheckBox *Checkbox_point=new QCheckBox;
  if (Draw_point) Checkbox_point->setChecked(true);
  _button_color_simple *Button_point=new _button_color_simple(QColor(int(Color_point.r*255.0f),int(Color_point.g*255.0f),int(Color_point.b*255.0f)));

  Grid_layout_point->addWidget(Checkbox_point,0,0,Qt::AlignCenter);
  Grid_layout_point->addWidget(Button_point,0,1,Qt::AlignCenter);

  Groupbox_point->setLayout(Grid_layout_point);

  connect(Checkbox_point,SIGNAL(stateChanged(int)),this,SLOT(activate_point_slot(int)));
  connect(Button_point,SIGNAL(click_button(QColor)),this,SLOT(color_point_slot(QColor)));

  // line
  QGroupBox *Groupbox_line=new QGroupBox(tr("Line"));
  Groupbox_line->setAlignment(Qt::AlignCenter);
  QGridLayout *Grid_layout_line = new QGridLayout;
  QCheckBox *Checkbox_line=new QCheckBox;
  if (Draw_line) Checkbox_line->setChecked(true);
  _button_color_simple *Button_line=new _button_color_simple(QColor(int(Color_line.r*255.0f),int(Color_line.g*255.0f),int(Color_line.b*255.0f)));

  Grid_layout_line->addWidget(Checkbox_line,0,0,Qt::AlignCenter);
  Grid_layout_line->addWidget(Button_line,0,1,Qt::AlignCenter);

  Groupbox_line->setLayout(Grid_layout_line);

  connect(Checkbox_line,SIGNAL(stateChanged(int)),this,SLOT(activate_line_slot(int)));

  // fill
  QGroupBox *Groupbox_fill=new QGroupBox(tr("Fill"));
  Groupbox_fill->setAlignment(Qt::AlignCenter);
  QGridLayout *Grid_layout_fill = new QGridLayout;
  QCheckBox *Checkbox_fill=new QCheckBox;
  if (Draw_fill) Checkbox_fill->setChecked(true);
  _button_color_simple *Button_fill=new _button_color_simple(QColor(int(Color_fill.r*255.0f),int(Color_fill.g*255.0f),int(Color_fill.b*255.0f)));

  Grid_layout_fill->addWidget(Checkbox_fill,0,0,Qt::AlignCenter);
  Grid_layout_fill->addWidget(Button_fill,0,1,Qt::AlignCenter);

  Groupbox_fill->setLayout(Grid_layout_fill);

  connect(Checkbox_fill,SIGNAL(stateChanged(int)),this,SLOT(activate_fill_slot(int)));
  connect(Button_fill,SIGNAL(click_button(QColor)),this,SLOT(color_fill_slot(QColor)));

  // Parameters
  QGroupBox *Groupbox_parameters=new QGroupBox(tr("Parameters"));
  Groupbox_parameters->setAlignment(Qt::AlignCenter);

  QVBoxLayout *Vboxlayout_parameters=new QVBoxLayout;

  // lighting and texturing
  QWidget *Widget_lighting_texturing=new QWidget;
  QGridLayout *Grid_layout_lighting_texturing = new QGridLayout;

  QLabel *Label_lighting=new QLabel("Lighting");
  QLabel *Label_texturing=new QLabel("Texturing");

  QCheckBox *Checkbox_lighting=new QCheckBox;
  if (Lighting) Checkbox_lighting->setChecked(true);

  QCheckBox *Checkbox_texturing=new QCheckBox;
  if (Texturing) Checkbox_texturing->setChecked(true);

  Grid_layout_lighting_texturing->addWidget(Label_lighting,0,0,Qt::AlignLeft);
  Grid_layout_lighting_texturing->addWidget(Checkbox_lighting,0,1,Qt::AlignCenter);
  Grid_layout_lighting_texturing->addWidget(Label_texturing,1,0,Qt::AlignLeft);
  Grid_layout_lighting_texturing->addWidget(Checkbox_texturing,1,1,Qt::AlignCenter);

  Widget_lighting_texturing->setLayout(Grid_layout_lighting_texturing);

  connect(Checkbox_lighting,SIGNAL(stateChanged(int)),this,SLOT(activate_lighting_slot(int)));
  connect(Checkbox_texturing,SIGNAL(stateChanged(int)),this,SLOT(activate_texturing_slot(int)));

  // Flat and smooth
  QWidget *Widget_shading=new QWidget;
  QGridLayout *Grid_layout_shading = new QGridLayout;

  QLabel *Label_shading_flat=new QLabel("Flat shading");
  QLabel *Label_shading_smooth=new QLabel("Smooth shading");

  Radiobutton_shading_flat=new QRadioButton;
  if (Shading_flat) Radiobutton_shading_flat->setChecked(true);

  QRadioButton *Radiobutton_shading_smooth=new QRadioButton;

  Grid_layout_shading->addWidget(Label_shading_flat,0,0,Qt::AlignLeft);
  Grid_layout_shading->addWidget(Radiobutton_shading_flat,0,1,Qt::AlignCenter);
  Grid_layout_shading->addWidget(Label_shading_smooth,1,0,Qt::AlignLeft);
  Grid_layout_shading->addWidget(Radiobutton_shading_smooth,1,1,Qt::AlignCenter);

  Widget_shading->setLayout(Grid_layout_shading);

  connect(Radiobutton_shading_flat,SIGNAL(toggled(bool)),this,SLOT(activate_shading_slot(bool)));
  // mode color
  QWidget *Widget_mode_color=new QWidget;
  QGridLayout *Grid_layout_mode_color = new QGridLayout;

  QLabel *Label_mode_color_constant=new QLabel("Color constant");
  QLabel *Label_mode_color_triangle=new QLabel("Color triangle");
  QLabel *Label_mode_color_vertex=new QLabel("Color vertex");

  for (uint i=0;i<3;i++){
    Radiobutton_mode_color[i]=new QRadioButton;
    connect(Radiobutton_mode_color[i],SIGNAL(toggled(bool)),this,SLOT(activate_mode_color_slot(bool)));
  }

  Radiobutton_mode_color[Mode_color]->setChecked(true);

  Grid_layout_mode_color->addWidget(Label_mode_color_constant,0,0,Qt::AlignLeft);
  Grid_layout_mode_color->addWidget(Radiobutton_mode_color[0],0,1,Qt::AlignCenter);
  Grid_layout_mode_color->addWidget(Label_mode_color_triangle,1,0,Qt::AlignLeft);
  Grid_layout_mode_color->addWidget(Radiobutton_mode_color[1],1,1,Qt::AlignCenter);
  Grid_layout_mode_color->addWidget(Label_mode_color_vertex,2,0,Qt::AlignLeft);
  Grid_layout_mode_color->addWidget(Radiobutton_mode_color[2],2,1,Qt::AlignCenter);

  Widget_mode_color->setLayout(Grid_layout_mode_color);

  // separator
  QFrame *Separator1=new QFrame();
  Separator1->setFrameStyle(QFrame::HLine);

  QFrame *Separator2=new QFrame();
  Separator2->setFrameStyle(QFrame::HLine);

  Vboxlayout_parameters->addWidget(Widget_lighting_texturing);
  Vboxlayout_parameters->addWidget(Separator1);
  Vboxlayout_parameters->addWidget(Widget_shading);
  Vboxlayout_parameters->addWidget(Separator2);
  Vboxlayout_parameters->addWidget(Widget_mode_color);

  Groupbox_parameters->setLayout(Vboxlayout_parameters);

  // General
  Vertical_layout_view->addWidget(Groupbox_background);
  Vertical_layout_view->addWidget(Groupbox_point);
  Vertical_layout_view->addWidget(Groupbox_line);
  Vertical_layout_view->addWidget(Groupbox_fill);
  Vertical_layout_view->addWidget(Groupbox_parameters);
  Vertical_layout_view->addStretch();

  Widget_view->setLayout(Vertical_layout_view);
}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::update_info()
{
  Table_info->set_valuei(0,int(Object3D->Vertices.size()));
  Table_info->set_valuei(1,int(Object3D->Triangles.size()));
  Table_info->set_valuei(2,int(Parametrizer.Vector_patches.size()));
  Table_info->set_valuef(3,Parametrizer.total_error());
  Table_info->set_valuef(4,Parametrizer.mean_error());
  Table_info->set_valuef(5,Parametrizer.max_error());
  Table_info->set_valuef(6,Parametrizer.time_parametrization());
  Table_info->set_valuef(7,Parametrizer.time_packing());

}


/*****************************************************************************//**
 *
 *
 *
 *****************************************************************************/

void _gl_widget::pick()
{
  QMatrix4x4 Projection;
  QMatrix4x4 View;
  QMatrix4x4 Model;
  float Distance1=Camera_data[Mode_visualization].Distance;

  float Aspect=float(Window_height)/float(Window_width);

  if (Mode_visualization==0){
    if (Projection_type==PERSPECTIVE_PROJECTION){
      Projection.frustum(-Distance1,Distance1,-Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PERSPECTIVE,BACK_PLANE_PERSPECTIVE);
    }
    else{
      Projection.ortho(-Distance1,Distance1,Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PARALLEL,BACK_PLANE_PARALLEL);
    }

    View.translate(0,0,-Distance);
    View.rotate(Angle_camera_x,1,0,0);
    View.rotate(Angle_camera_y,0,1,0);
    View.translate(Camera_data[Mode_visualization].Traslation.x,Camera_data[Mode_visualization].Traslation.y);
  }
  else{
    Projection.ortho(-Distance1,Distance1,-Distance1*Aspect,Distance1*Aspect,FRONT_PLANE_PARALLEL,BACK_PLANE_PARALLEL);

    View.translate(0,0,-Distance);
    View.rotate(0,1,0,0);
    View.rotate(0,0,1,0);
    View.translate(Camera_data[Mode_visualization].Traslation.x,Camera_data[Mode_visualization].Traslation.y);

    View.translate(-DEFAULT_TEXTURE_TRANSLATION_FACTOR,-DEFAULT_TEXTURE_TRANSLATION_FACTOR);
    View.scale(DEFAULT_TEXTURE_SCALE_FACTOR,DEFAULT_TEXTURE_SCALE_FACTOR,1.0);
  }

  makeCurrent();

  // Frame Buffer Object to do the off-screen rendering
  glGenFramebuffers(1,&FBO);
  glBindFramebuffer(GL_FRAMEBUFFER,FBO);

  // Texture for drawing
  glGenTextures(1,&Color_texture);
  glBindTexture(GL_TEXTURE_2D,Color_texture);
  // RGBA8
  glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA8, Window_width,Window_height);
  // this implies that there is not mip mapping
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  // Texure for computing the depth
  glGenTextures(1,&Depth_texture);
  glBindTexture(GL_TEXTURE_2D,Depth_texture);
  // Float
  glTexStorage2D(GL_TEXTURE_2D,1,GL_DEPTH_COMPONENT24, Window_width,Window_height);

  // Attatchment of the textures to the FBO
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,Color_texture,0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,Depth_texture,0);

  // OpenGL will draw to these buffers (only one in this case)
  static const GLenum Draw_buffers[]={GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1,Draw_buffers);

  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glUseProgram(Program_pick);
  if (Mode_visualization==0) glBindVertexArray(VAO1);
  else glBindVertexArray(VAO2);

  glUniformMatrix4fv(10,1,GL_FALSE,Model.data());
  glUniformMatrix4fv(11,1,GL_FALSE,View.data());
  glUniformMatrix4fv(12,1,GL_FALSE,Projection.data());

  // draw the model
  if (Mode_visualization==0) Object3D->draw();
  else Texture_object->draw();

  // get the pixel
  int Color;
  glReadBuffer(GL_FRONT);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glReadPixels(Selection_position_x,Selection_position_y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&Color);

  uint B=uint((Color & 0x00FF0000) >> 16);
  uint G=uint((Color & 0x0000FF00) >> 8);
  uint R=uint((Color & 0x000000FF));
//  cout << "Selected=" << R << " " << G << " " << B << endl;

  Selected_triangle= int((R << 16) + (G << 8) + B);

  if (Selected_triangle==16777215){
    Selected_triangle=-1;
    Selected_triangle1=-1;
    Selected_triangle2=-1;
    Selected_triangle3=-1;
  }
  else{
    Object3D->get_triangles(Selected_triangle,Selected_triangle1,Selected_triangle2,Selected_triangle3);
  }

  glUseProgram(0);
  glBindVertexArray(0);

  glDeleteTextures(1,&Color_texture);
  glDeleteTextures(1,&Depth_texture);
  glDeleteFramebuffers(1,&FBO);
  // the normal framebuffer take the control of drawing
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,defaultFramebufferObject());
}
