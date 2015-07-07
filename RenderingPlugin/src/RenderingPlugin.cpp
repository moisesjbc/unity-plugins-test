// Example low level rendering Unity plugin

#include <RenderingPlugin.h>

#include <math.h>
#include <stdio.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <string>
#include <fstream>

// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#if !SUPPORT_OPENGL
    #error "OpenGL required!"
#endif

#if UNITY_WIN || UNITY_LINUX
    #include <GL/gl.h>
#else
    #include <GLUT/GLUT.h>
    #include <OpenGL/OpenGL.h>
#endif



// --------------------------------------------------------------------------
// Helper utilities

// Prints a string
static void DebugLog (const char* str)
{
	#if UNITY_WIN
	OutputDebugStringA (str);
	#else
	printf ("%s", str);
	#endif
}

// COM-like Release macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if (a) { a->Release(); a = NULL; }
#endif

static std::string openGLErrorsLog;

extern "C" const char* getOpenGLErrorsLog()
{
    return openGLErrorsLog.c_str();
}


#define OPENGL_ERROR_CASE(str,errorCode) case(errorCode): str=#errorCode; break;

void checkOpenGLStatus( const char* situation )
{
    std::ofstream logFile( "rendering-plugin-log.txt", std::ofstream::out | std::ofstream::app );
    GLenum errorCode = glGetError();
    std::string errorMessage;
    switch( errorCode ){
        OPENGL_ERROR_CASE( errorMessage, GL_NO_ERROR );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_ENUM );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_VALUE );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_OPERATION );
        OPENGL_ERROR_CASE( errorMessage, GL_INVALID_FRAMEBUFFER_OPERATION );
        OPENGL_ERROR_CASE( errorMessage, GL_OUT_OF_MEMORY );
        OPENGL_ERROR_CASE( errorMessage, GL_STACK_UNDERFLOW );
        OPENGL_ERROR_CASE( errorMessage, GL_STACK_OVERFLOW );
        default:
            errorMessage = "Unknown error";
        break;
    }
        
    logFile << errorMessage.c_str() << " at " << situation << "\n";
    
    logFile.close();
}


// --------------------------------------------------------------------------
// SetTimeFromUnity, an example function we export which is called by one of the scripts.

static float g_Time;

void EXPORT_API SetTimeFromUnity (float t)
{
    g_Time = t;
}

static glm::mat4 modelMatrix_;
static glm::mat4 viewMatrix_;
static glm::mat4 projectionMatrix_;
static glm::vec4 cameraPos_;

void EXPORT_API SetMatricesFromUnity( float* modelMatrix,
                                        float* viewMatrix,
                                        float* projectionMatrix )
{
    modelMatrix_ = glm::make_mat4( modelMatrix );
    viewMatrix_ = glm::make_mat4( viewMatrix );
    projectionMatrix_ = glm::make_mat4( projectionMatrix );
    cameraPos_ = glm::inverse( viewMatrix_ ) * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
}


// --------------------------------------------------------------------------
// shaders

#define VPROG_SRC(ver, attr, varying)								\
ver																\
attr " vec3 pos;\n"										\
attr " vec4 color;\n"										\
"\n"															\
varying " vec4 ocolor;\n"									\
"\n"															\
"uniform mat4 worldMatrix;\n"								\
"uniform mat4 projMatrix;\n"								\
"\n"															\
"void main()\n"													\
"{\n"															\
"	gl_Position = (projMatrix * worldMatrix) * vec4(pos,1);\n"	\
"	ocolor = color;\n"											\
"}\n"															\

static const char* kGlesVProgTextGLES2		= VPROG_SRC("\n", "attribute", "varying");
static const char* kGlesVProgTextGLES3		= VPROG_SRC("#version 300 es\n", "in", "out");

#undef VPROG_SRC

#define FSHADER_SRC(ver, varying, outDecl, outVar)	\
ver												\
outDecl											\
varying " vec4 ocolor;\n"					\
"\n"											\
"void main()\n"									\
"{\n"											\
"	" outVar " = ocolor;\n"						\
"}\n"											\

static const char* kGlesFShaderTextGLES2	= FSHADER_SRC("\n", "varying", "\n", "gl_FragColor");
static const char* kGlesFShaderTextGLES3	= FSHADER_SRC("#version 300 es\n", "in", "out lowp vec4 fragColor;\n", "fragColor");

#undef FSHADER_SRC

static GLuint	g_VProg;
static GLuint	g_FShader;
static GLuint	g_Program = 0;
static int		g_WorldMatrixUniformIndex;
static int		g_ProjMatrixUniformIndex;

static GLuint CreateShader(GLenum type, const char* text)
{
    GLuint ret = glCreateShader(type);
    glShaderSource(ret, 1, &text, NULL);
    glCompileShader(ret);
    
    GLint result;
    glGetShaderiv( ret, GL_COMPILE_STATUS, &result );
    
    std::ofstream logFile( "rendering-plugin-log.txt", std::ofstream::out | std::ofstream::app );
    logFile << "Shader: " << std::endl << std::endl << text << std::endl << std::endl;
    logFile << "Shader compiler status: " << result << std::endl;
    if( !result ){
        GLchar errorLog[1024] = {0};
        glGetShaderInfoLog(ret, 1024, NULL, errorLog);
        logFile << errorLog << std::endl;
    }
    logFile.close();
    
    return ret;
}


// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

static void*	g_TexturePointer	= 0;
static int		g_TexWidth			= 0;
static int		g_TexHeight			= 0;

void EXPORT_API SetTextureFromUnity(void* texturePtr, int w, int h)
{
    g_TexturePointer	= texturePtr;
    g_TexWidth			= w;
    g_TexHeight			= h;
}


// --------------------------------------------------------------------------
// UnitySetGraphicsDevice

static int g_DeviceType = -1;

void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
    // Truncate log file.
    std::ofstream logFile( "rendering-plugin-log.txt" );
    logFile.close();
    
    DebugLog("OpenGLES 2.0 device\n");
    ::printf("OpenGLES 2.0 device\n");
    checkOpenGLStatus( "UnitySetGraphicsDevice - 1" );
    
    g_VProg		= CreateShader(GL_VERTEX_SHADER, kGlesVProgTextGLES2);
    g_FShader	= CreateShader(GL_FRAGMENT_SHADER, kGlesFShaderTextGLES2);
    checkOpenGLStatus( "UnitySetGraphicsDevice - 2" );
    
    g_Program = glCreateProgram();
    logFile.open( "rendering-plugin-log.txt", std::ofstream::out | std::ofstream::app );
    logFile << "g_Program: " << g_Program << std::endl;
    logFile.close();
    
    glBindAttribLocation(g_Program, 0, "pos");
    glBindAttribLocation(g_Program, 1, "color");
    glAttachShader(g_Program, g_VProg);
    glAttachShader(g_Program, g_FShader);
    glLinkProgram(g_Program);
    int result;
    
    glGetProgramiv( g_Program, GL_LINK_STATUS, &result );
    logFile.open( "rendering-plugin-log.txt", std::ofstream::out | std::ofstream::app );
    logFile << "Shader link status: " << result << std::endl;
    if( !result ){
        GLchar errorLog[1024] = {0};
        glGetProgramInfoLog(g_Program, 1024, NULL, errorLog);
        logFile << errorLog << std::endl;
    }
    logFile.close();
    
    glGetProgramiv( g_Program, GL_ATTACHED_SHADERS, &result );
    logFile.open( "rendering-plugin-log.txt", std::ofstream::out | std::ofstream::app );
    logFile << "Attached shaders status: " << result << std::endl;
    logFile.close();
    
    checkOpenGLStatus( "UnitySetGraphicsDevice - 3" );
    
    g_WorldMatrixUniformIndex	= glGetUniformLocation(g_Program, "worldMatrix");
    g_ProjMatrixUniformIndex	= glGetUniformLocation(g_Program, "projMatrix");
    checkOpenGLStatus( "UnitySetGraphicsDevice - 4" );
    
    g_DeviceType = deviceType;
}



// --------------------------------------------------------------------------
// UnityRenderEvent
// This will be called for GL.IssuePluginEvent script calls; eventID will
// be the integer passed to IssuePluginEvent. In this example, we just ignore
// that value.


struct MyVertex {
	float x, y, z;
	unsigned int color;

    MyVertex() : x(0.0f), y(0.0f), z(0.0f), color(0) {}
    MyVertex( float x, float y, float z, unsigned int color ) :
        x(x),
        y(y),
        z(z),
        color(color)
    {}
};
static void SetDefaultGraphicsState ();
static void DoRendering (const glm::mat4& worldMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& projectionMatrix,
                         const MyVertex* verts);

void subdividePlane( const MyVertex* planeVertices,
                    std::vector<MyVertex>& vertices )
{
    MyVertex middleVertices[4];
    for( int i = 0; i < 4; i++ ){
        const MyVertex& currentVertex = planeVertices[i];
        const MyVertex& nextVertex = planeVertices[(i+1)%4];
        const MyVertex middleVertex(
                                    ( currentVertex.x + nextVertex.x ) / 2.0f,
                                    ( currentVertex.y + nextVertex.y ) / 2.0f,
                                    ( currentVertex.z + nextVertex.z ) / 2.0f,
                                    ( currentVertex.color + nextVertex.color ) / 2.0f
                                    );

        middleVertices[i] = middleVertex;
    }
    // Compute plane centroid
    MyVertex planeCentroid;
    for( int i = 0; i < 4; i++ )
    {
        planeCentroid.x += planeVertices[i].x;
        planeCentroid.y += planeVertices[i].y;
        planeCentroid.z += planeVertices[i].z;
        planeCentroid.color += planeVertices[i].color;
    }
    planeCentroid.x /= 4.0f;
    planeCentroid.y /= 4.0f;
    planeCentroid.z /= 4.0f;
    planeCentroid.color /= 4.0f;

    // Subplane 0
    vertices.push_back( middleVertices[3] );
    vertices.push_back( planeVertices[0] );
    vertices.push_back( middleVertices[0] );
    vertices.push_back( planeCentroid );

    // Subplane 1
    vertices.push_back( middleVertices[0] );
    vertices.push_back( planeVertices[1] );
    vertices.push_back( middleVertices[1] );
    vertices.push_back( planeCentroid );

    // Subplane 2
    vertices.push_back( middleVertices[1] );
    vertices.push_back( planeVertices[2] );
    vertices.push_back( middleVertices[2] );
    vertices.push_back( planeCentroid );

    // Subplane 3
    vertices.push_back( middleVertices[2] );
    vertices.push_back( planeVertices[3] );
    vertices.push_back( middleVertices[3] );
    vertices.push_back( planeCentroid );
}

std::vector< MyVertex > vertices;

void EXPORT_API InitPlugin()
{
    // A plane.
    MyVertex srcVertices[] =
    {
        MyVertex( -0.5f, 0.0f, -0.5f, 0xFF00ff00 ),
        MyVertex( 0.5f, 0.0f, -0.5f, 0xFF00ff00 ),
        MyVertex(  0.5f, 0.0f, 0.5f, 0xFF00ff00 ),
        MyVertex( -0.5f, 0.0f, 0.5f, 0xFF00ff00 )
    };

    // Copy original plane to vertices vector
    for( int i = 0; i < 4; i++ ){
        vertices.push_back( srcVertices[i] );
    }

    // Subdivide plane (2).
    subdividePlane( &vertices[0], vertices );

    // Subdivide plane (3).
    subdividePlane( &vertices[4], vertices );
    subdividePlane( &vertices[8], vertices );
    subdividePlane( &vertices[12], vertices );
    subdividePlane( &vertices[16], vertices );
}


void EXPORT_API UnityRenderEvent (int eventID)
{
	// Unknown graphics device type? Do nothing.
	if (g_DeviceType == -1)
		return;

	// Actual functions defined below
	SetDefaultGraphicsState ();
	DoRendering( modelMatrix_, viewMatrix_, projectionMatrix_, vertices.data() );
}


// --------------------------------------------------------------------------
// SetDefaultGraphicsState
//
// Helper function to setup some "sane" graphics state. Rendering state
// upon call into our plugin can be almost completely arbitrary depending
// on what was rendered in Unity before.
// Before calling into the plugin, Unity will set shaders to null,
// and will unbind most of "current" objects (e.g. VBOs in OpenGL case).
//
// Here, we set culling off, lighting off, alpha blend & test off, Z
// comparison to less equal, and Z writes off.

static void SetDefaultGraphicsState ()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}


static void FillTextureFromCode (int width, int height, int stride, unsigned char* dst)
{
	const float t = g_Time * 4.0f;

	for (int y = 0; y < height; ++y)
	{
		unsigned char* ptr = dst;
		for (int x = 0; x < width; ++x)
		{
			// Simple oldskool "plasma effect", a bunch of combined sine waves
			int vv = int(
				(127.0f + (127.0f * sinf(x/7.0f+t))) +
				(127.0f + (127.0f * sinf(y/5.0f-t))) +
				(127.0f + (127.0f * sinf((x+y)/6.0f-t))) +
				(127.0f + (127.0f * sinf(sqrtf(float(x*x + y*y))/4.0f-t)))
				) / 4;

			// Write the texture pixel
			ptr[0] = vv;
			ptr[1] = vv;
			ptr[2] = vv;
			ptr[3] = vv;

			// To next pixel (our pixels are 4 bpp)
			ptr += 4;
		}

		// To next image row
		dst += stride;
	}
}


static void DoRendering ( const glm::mat4& modelMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& projectionMatrix,
                         const MyVertex* verts)
{
        // Set shader program
        glUseProgram(g_Program);
        
        // Send modelview matrix to shader.
        const glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glUniformMatrix4fv(g_WorldMatrixUniformIndex, 1, GL_FALSE, glm::value_ptr( modelViewMatrix ) );
        
        // Send projection matrix to shader.
        glUniformMatrix4fv(g_ProjMatrixUniformIndex, 1, GL_FALSE, glm::value_ptr( projectionMatrix ) );
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Vertex layout.
        const int stride = 3*sizeof(float) + sizeof(unsigned int);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const float*)verts);
        
        glEnableVertexAttribArray(1);
        glVertexAttribIPointerEXT(1, 4, GL_UNSIGNED_BYTE, stride, (const float*)verts + 3);

        // Compute the distance between the camera and the plane.
        // TODO: Compute real distance.
        const float distance = glm::length( cameraPos_ );

        // Draw a version of the plane or another depending on the distance between
        // the camera and the plane.
        if( distance > 3.0f ){
            glDrawArrays (GL_QUADS, 0, 4);
        }else if( distance > 2.0f ){
            glDrawArrays( GL_QUADS, 4, 16 );
        }else{
            glDrawArrays( GL_QUADS, 20, 64 );
        }

        // update native texture from code
        if (g_TexturePointer)
        {
            GLuint gltex = (GLuint)(size_t)(g_TexturePointer);
            glBindTexture(GL_TEXTURE_2D, gltex);
            
            unsigned char* data = new unsigned char[g_TexWidth*g_TexHeight*4];
            FillTextureFromCode(g_TexWidth, g_TexHeight, g_TexHeight*4, data);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_TexWidth, g_TexHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
            delete[] data;
        }
}