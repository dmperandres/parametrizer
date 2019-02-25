# parametrizer

This is my code of Sorkine's method for parametrizing 3D models. I have implemented it for our current project. It needs Qt library. Also, the project is for Qt.

The main parts are:
- _object3D: This class creates all the needed information from a PLY file
- _parametrizer: This clas implements Sorkine's method. The class _patch does most of the work. The method is not optimized
- _rectangle_packer: This class implments the packing of all patches in a square texture. Given that for our project it is not important the orientation, it is possible to rate the rectancles that include the patches to better use the space. The method is very simple but very fast.
- _gl_widget: This class does all the initializacion and visualization.


