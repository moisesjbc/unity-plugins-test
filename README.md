# unity-plugins-test

This is a test about creating a rendering plugin for [Unity](http://unity3d.com). When used together, we get a scene with some models rendered by Unity and some others rendered by the plugin itself.

**NOTE**: At this point of the development, this project is being built **under Windows only**.

## Dependencies

* [Unity (>= 5)](http://unity3d.com)
* [Git for windows](https://msysgit.github.io/)
* [CMake (>= 2.8)](http://www.cmake.org)
* A C++ compiler of your preference. I am using [MinGW-w64](http://sourceforge.net/projects/mingw-w64/).
* An IDE of your preference. I am using [Qt Creator](https://www.qt.io/download-open-source/).
* [GLM (>= 0.9.6.3](glm.g-truc.net/)
* [GLEW (>= 1.12.0](http://glew.sourceforge.net/)
* [Easylogging++](https://github.com/easylogging/easyloggingpp)

### Android dependencies

* [Java SE JDK 8](http://www.oracle.com/technetwork/es/java/javase/downloads/index.html)
* [Android SDK (tools only) (>= v24)](https://developer.android.com/sdk/index.html)
	* Once installed, execute Android SDK Manager as administrator and install "Android  4.0.3 (API 15)" images.
* [Android NDK (>= r10)](https://developer.android.com/ndk/downloads/index.html)
* [Make for Windows](http://gnuwin32.sourceforge.net/packages/make.htm)

## Building and running the project

1. Install all the dependencies listed in previous section (**except for Easyloggin++**, which is already included in this repository).

2. Clone this repository. This will create a local directory which we’ll refer to as *LOCAL_REPO_DIR* in the following steps.

 ```
 git clone git@github.com:moisesjbc/unity-plugins-test.git
 ```

3. Run cmake-gui on the directory *LOCAL_REPO_DIR*/RenderingPlugin.

4. Generate a project for Visual Studio, Xcode, or your preferred build tool.

5. Close cmake-gui and open the project generated in the previous step.

6. Build the plugin and copy it to *LOCAL_REPO_DIR*/UnityProject/Assets/Plugins

7. Now run Unity **forcing it to use OpenGL**

 ```
 "C:\Program Files\Unity\Editor\Unity.exe" -force-opengl
 ```

8. From Unity, open the project *LOCAL_REPO_DIR*/UnityProject

9. Press play and enjoy this masterpiece!

## Working on the project

### Directory hierarchy

The **main files** of the project’s directory hierarchy are presented below:

*LOCAL_REPO_DIR*

* **RenderingPlugin/:** Code for Plugin’s rendering plugin.
  * **CMakeLists.txt:** Specification file for building the plugin using CMake
  * **include/:** Headers
    * **RenderingPlugin.h:** Declarations of exported functions to be called by Unity.
    * **\*.hpp:** Declarations of inner classes used by the plugin.
  * **src/:** Source files
    * **RenderingPlugin.c:** Definitions of exported functions to be called by Unity.
    * **\*.cpp:** Definitions of inner classes used by the plugin.
* **UnityProject/:** Unity project using the rendering plugin.
  * **Assets/:** Project assets
    * **Scripts/:** Project scripts. Some of these scripts are the ones which communicates with the rendering plugin.

### IMPORTANT NOTE FOR DEVELOPERS

When recompiling the plugin, this must be moved to the *LOCAL_REPO_DIR*/UnityProject/Assets/Plugins directory in order for Unity to use it. **But there is a problem**, according to [this link](http://answers.unity3d.com/questions/55234/recompiled-plugins-not-refreshing-in-unity.html), **"On Windows, the DLL file would be locked for writing, stopping you from overwiting it"**. This means that, in order to update the plugin, the Unity project must be closed while we overwrite the old version of the plugin with the new one. Once we reopen Unity project, it should load the new version of the plugin.

### Editing the plugin’s inner code

When editing variables or functions **which aren’t called directly by Unity**, simply make the desired change and recompile the plugin.

**NOTE: ** if your changes affect the CMakeLists.txt specification (ie. removing source files, adding libraries, etc), please edit CMakeLists.txt so the project can be built from zero with the new changes :)

### Altering the communication Unity - Plugin

Let’s suppose that you have added a C++ function with the following signature

```C++
    int add( int a, int b );
```

to the plugin, and you want to call that function from Unity. For that, make the following:

1. Make sure that your function’s declaration is inside an extern “C” block and that it includes the EXPORT_API macro.

    ```C++
    #include <RenderingPlugin.h>

    extern “C” {
        int EXPORT_API add( int a, int b );
    }
    ```

2. From Unity, open the script file you want to call the function from (ie. UnityProject/UseRenderingPlugin.cs). Then, “import” your function.

    ```C#
    public class UseRenderingPlugin : MonoBehaviour
    {
        #if UNITY_IPHONE && !UNITY_EDITOR
	    [DllImport ("__Internal")]
        #else
	    [DllImport ("RenderingPlugin")]
        #endif
        …
        private static extern int add( int a, int b );
        …
    }
    ```

3. Now you can use your awesome plugin function from Unity!

    ```C#
    IEnumerator Start () {
        …
        Debug.Log( "3 + 5 = " + add ( 3, 5 )  );
        …
    }
    ```
