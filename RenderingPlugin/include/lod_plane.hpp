#ifndef LOD_PLANE_HPP
#define LOD_PLANE_HPP

#include <RenderingPlugin.h>

#include <easylogging++.h>

// Include headers for the graphics APIs we support

#if !SUPPORT_OPENGL
#error "OpenGL required!"
#endif

#define GL_GLEXT_PROTOTYPES
#if UNITY_WIN || UNITY_LINUX
    #include <windows.h>
	#define GLEW_STATIC
    #include <GL/glew.h>
#elif UNITY_IPHONE
    #include <OpenGLES/ES2/gl.h>
#elif UNITY_ANDROID
    #include <GLES2/gl2.h>
#else
    // TODO: can't include GLES2 on Mac?
    // (http://forums.macrumors.com/threads/xcode-opengl-es2-desktop-project.1109285/)
    #include <OpenGL/gl3.h>
#endif

#include <vector>

struct MyVertex {
    float x, y, z;
    unsigned int color;
	float uvX, uvY;
    
    MyVertex() : x(0.0f), y(0.0f), z(0.0f), color(0), uvX(0.0f), uvY(0.0f) {}
    MyVertex( float x, float y, float z, unsigned int color, float uvX, float uvY ) :
    x(x),
    y(y),
    z(z),
    color(color),
	uvX(uvX),
	uvY(uvY)
    {}
};

class LODPlane {
    public:
        LODPlane();
    
        void render( float distanceToObserver );
    
    private:
        void subdividePlane( std::vector< MyVertex >& vertices,
                            std::vector< GLubyte>& indices,
                            unsigned int planeFirstVertexIndex );
    
        std::vector< MyVertex > vertices_;
        std::vector< GLubyte > indices_;
};

#endif 
// LOD_PLANE_HPP