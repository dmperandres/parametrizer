
HEADERS += \
  vertex.h  \
  accuracy.h \
  rectangle_packer.h \
  table_info.h \
  object3D.h \
  parametrizador.h \
  button_color_simple.h \
  axis.h \
  material.h \
  basic_object3d.h \
  texture_object.h \
  simple_texture_object.h \
  file_ply_stl.h \
  shaders.h \
  material.h \
  glwidget.h \
  window.h

SOURCES += \
  accuracy.cc \
  table_info.cc \
  rectangle_packer.cc \
  object3D.cc \
  parametrizador.cc \
  button_color_simple.cc \
  axis.cc \
  basic_object3d.cc \
  texture_object.cc \
  simple_texture_object.cc \
  file_ply_stl.cc \
  shaders.cc \
  main.cc \
#  material.cc \
  glwidget.cc \
  window.cc

INCLUDEPATH += /XXXXX/glew/include

LIBS += -L/usr/X11R6/lib64 -lGL \
        -L/XXXXX/glew/lib -lGLEW

CONFIG += c++14
QT += opengl
QT += widgets
