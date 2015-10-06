#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <IUnityGraphics.h>

// Which platform we are on?
#if _MSC_VER || __MINGW32__
#define UNITY_WIN 1
#elif defined(__APPLE__)
    #if defined(__arm__)
        #define UNITY_IPHONE 1
    #else
        #define UNITY_OSX 1
    #endif
#elif defined(__linux__)
#define UNITY_LINUX 1
#elif defined(UNITY_METRO) || defined(UNITY_ANDROID)
// these are defined externally
#else
#error "Unknown platform!"
#endif


// Attribute to make function be exported from a plugin
#if UNITY_METRO
#define EXPORT_API __declspec(dllexport) __stdcall
#elif UNITY_WIN
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif


// Which graphics device APIs we possibly support?
#if UNITY_WIN
#define SUPPORT_OPENGL 1
#endif

#if UNITY_OSX || UNITY_LINUX
#define SUPPORT_OPENGL 1
#endif

#if UNITY_IPHONE || UNITY_ANDROID
    #define SUPPORT_OPENGLES 1
#endif

// Include headers for the graphics APIs we support

#if !SUPPORT_OPENGL
#error "OpenGL required!"
#endif

#include <easylogging++.h>

#define GL_GLEXT_PROTOTYPES
#if !__ANDROID__ && (UNITY_WIN || UNITY_LINUX)
    #include <windows.h>
    #define GLEW_STATIC
    #include <GL/glew.h>
#elif UNITY_IPHONE
    #include <OpenGLES/ES2/gl.h>
#elif UNITY_ANDROID || __ANDROID__
    #include <GLES2/gl2.h>
#else
    // TODO: can't include GLES2 on Mac?
    // (http://forums.macrumors.com/threads/xcode-opengl-es2-desktop-project.1109285/)
    #include <OpenGL/gl3.h>
#endif


#define OPENGL_ERROR_CASE(str,errorCode) case(errorCode): str=#errorCode; break;

inline void checkOpenGLStatus( const char* situation )
{
    GLenum errorCode = glGetError();
    std::string errorMessage;
    switch( errorCode ){
        OPENGL_ERROR_CASE( errorMessage, GL_NO_ERROR );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_ENUM );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_VALUE );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_OPERATION );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_FRAMEBUFFER_OPERATION );
        OPENGL_ERROR_CASE( errorMessage, GL_OUT_OF_MEMORY );
        default:
            errorMessage = "Unknown error";
        break;
    }

    LOG(INFO) << errorMessage.c_str() << " at " << situation << "\n";
}

#endif // PLATFORM_HPP

