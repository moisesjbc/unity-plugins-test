# unity-plugins-test

This is a test about creating a rendering plugin for [Unity](http://unity3d.com). When used together, we get a scene with some models rendered by Unity and some others rendered by the plugin itself.

## Dependencies

* [Unity (>= 5)](http://unity3d.com)
* [CMake (>= 2.8)](http://www.cmake.org)
* [OpenGL ES 2.0](https://www.khronos.org/opengles/2_X/)
* [GLUT](https://www.opengl.org/resources/libraries/glut/)
* [GLM (>= 0.9.6.3)](http://glm.g-truc.net/0.9.6/index.html)
* [SDL2 (>= 2.0)](https://www.libsdl.org)
* [SDL2_Image (>= 2.0)](https://www.libsdl.org/projects/SDL_image/)

## Building and running the project

1. Install all the dependencies listed in previous section.

2. Clone this repository. This will create a local directory which we’ll refer to as <local_repository> in the following steps.

 ```
 git clone git@github.com:moisesjbc/unity-plugins-test.git
 ```

3. Enter into the plugin’s directory

 ```
 cd <local_repository>/RenderingPlugin
 ```

4. Build the plugin using CMake and Make. This should generate a library in the folder <project_dir>/UnityProject/Assets/Plugins

 ```
 cmake -DCMAKE_MODULE_PATH=/usr/local/lib/cmake . && make
 ```

5. Run Unity and open the project <project_dir>/UnityProject

6. Press play and enjoy this masterpiece!