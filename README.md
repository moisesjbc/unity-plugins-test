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

2. Clone this repository. This will create a local directory which we’ll refer to as &lt;local_repository&gt; in the following steps.

 ```
 git clone git@github.com:moisesjbc/unity-plugins-test.git
 ```

3. Enter into the plugin’s directory

 ```
 cd <local_repository>/RenderingPlugin
 ```

4. Build the plugin using CMake and Make. This should generate a library in the folder &lt;project_dir&gt;/UnityProject/Assets/Plugins

 ```
 cmake -DCMAKE_MODULE_PATH=/usr/local/lib/cmake . && make
 ```

5. Run Unity and open the project <project_dir>/UnityProject

6. Press play and enjoy this masterpiece!

## Working on the project

### Directory hierarchy

The **main files** of the project’s directory hierarchy are presented below:

<local_repository>

* **RenderingPlugin/: ** Code for Plugin’s rendering plugin.
  * **CMakeLists.txt: ** Specification file for building the plugin using CMake
  * **include/: ** Headers
    * **RenderingPlugin.h: ** Declarations of exported functions to be called by Unity.
    * ** *.hpp: ** Declarations of inner classes used by the plugin.
  * **src/: ** Source files
    * **RenderingPlugin.c: ** Definitions of exported functions to be called by Unity.
    * ** *.cpp: ** Definitions of inner classes used by the plugin.
* **UnityProject/: ** Unity project using the rendering plugin.

### Note for Mac users

When working on Mac, I noticed that **recompiling the rendering plugin wouldn’t be enough for Unity to reload it**. If you have opened Unity and recompiled the rendering plugin. **The (cumbersome) solution is to reimport all assets from Unity**:

1. On Unity, go to the Project panel (on the bottom-left) and right-click on Assets.
2. Select “Reimport all”.
3. A “are you sure” dialog will popup. Click on “Reimport” button.
4. Unity will close and open again, with the updated version of the plugin loaded.

Reference: [Recompiled plugins not refreshing in Unity - Unity Answers](http://answers.unity3d.com/questions/55234/recompiled-plugins-not-refreshing-in-unity.html)

### Editing the plugin’s inner code

When editing variables or functions **which aren’t called directly by Unity**, simply make the desired change and recompile the plugin.