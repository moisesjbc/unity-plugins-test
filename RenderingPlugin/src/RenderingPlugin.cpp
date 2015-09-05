// Example low level rendering Unity plugin

#include <RenderingPlugin.h>

#include <math.h>
#include <stdio.h>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <string>
#include <fstream>
#include <lod_plane.hpp>

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

static GLuint	g_VProg;
static GLuint	g_FShader;
static GLuint	g_Program = 0;
static int		g_WorldMatrixUniformIndex;
static int		g_ProjMatrixUniformIndex;

static GLuint CreateShader(GLenum type, const char* text )
{
    checkOpenGLStatus( "CreateShader - 1" );

    LOG(INFO) << "Shader: " << std::endl << std::endl << text << std::endl << std::endl;
    GLuint ret = glCreateShader(type);

    checkOpenGLStatus( "CreateShader - 2" );
    glShaderSource( ret, 1, (const GLchar**)( &text ), nullptr );
    glCompileShader(ret);
    checkOpenGLStatus( "CreateShader - 3" );
    
    GLint result;
    glGetShaderiv( ret, GL_COMPILE_STATUS, &result );
    checkOpenGLStatus( "CreateShader - 4" );
    
    LOG(INFO) << "Shader compiler status: " << result << std::endl;
    if( !result ){
        GLchar errorLog[1024] = {0};
        glGetShaderInfoLog(ret, 1024, NULL, errorLog);
        LOG(INFO) << errorLog << std::endl;
    }
    checkOpenGLStatus( "CreateShader - 5" );
    
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


void LogOpenGLVersion()
{
    const GLubyte* oglVersion = glGetString( GL_VERSION );
    
	if( oglVersion ){
		LOG(INFO) << "oglVersion: " << oglVersion << std::endl;
	}
}

// --------------------------------------------------------------------------
// UnitySetGraphicsDevice

static int g_DeviceType = -1;

void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
	// Configure logger
#if !__ANDROID__
	el::Configurations defaultConf;
	defaultConf.setToDefault();
	defaultConf.set(el::Level::Info, el::ConfigurationType::Filename, "rendering-plugin-log.txt" );
	el::Loggers::reconfigureLogger("default", defaultConf);
#endif

	if ((deviceType != kGfxRendererOpenGL) && (deviceType != kGfxRendererOpenGLES20Mobile)){
		LOG(ERROR) << "NO OPENGL (" << deviceType << ")" << std::endl;
	}

#if !__ANDROID__
	if (glewInit() != GLEW_OK){
		LOG(ERROR) << "glewInit() failed" << std::endl;
	}
#endif

	checkOpenGLStatus("UnitySetGraphicsDevice - 0");
    
    LogOpenGLVersion();
    
    DebugLog("OpenGLES 2.0 device\n");
    ::printf("OpenGLES 2.0 device\n");
    checkOpenGLStatus( "UnitySetGraphicsDevice - 1" );

    std::ifstream shaderFile;
    char shaderCode[1024];
    
    shaderFile.open( "Assets/Shaders/basic_vertex.shader.txt" );
    if( shaderFile.is_open() ){
        shaderFile.read( shaderCode, 1024 );
        shaderCode[shaderFile.gcount()] = 0;
        g_VProg		= CreateShader(GL_VERTEX_SHADER, shaderCode);
        shaderFile.close();
    }else{
        LOG(INFO) << "Couldn't open shader file [Assets/Shaders/basic_vertex.shader]" << std::endl;
    }

    shaderFile.open( "Assets/Shaders/basic_fragment.shader.txt" );
    if( shaderFile.is_open() ){
        shaderFile.read( shaderCode, 1024 );
        shaderCode[shaderFile.gcount()] = 0;
        g_FShader	= CreateShader(GL_FRAGMENT_SHADER, shaderCode);
        shaderFile.close();
        checkOpenGLStatus( "UnitySetGraphicsDevice - 2" );
    }else{
        LOG(INFO) << "Couldn't open shader file [Assets/Shaders/basic_fragment.shader]" << std::endl;
    }

    g_Program = glCreateProgram();
	LOG(INFO) << "g_Program: " << g_Program << std::endl;
    
    glBindAttribLocation(g_Program, 0, "pos");
    glBindAttribLocation(g_Program, 1, "color");
    glAttachShader(g_Program, g_VProg);
    glAttachShader(g_Program, g_FShader);
    glLinkProgram(g_Program);
    int result;
    
    glGetProgramiv( g_Program, GL_LINK_STATUS, &result );
	LOG(INFO) << "Shader link status: " << result << std::endl;
    if( !result ){
        GLchar errorLog[1024] = {0};
        glGetProgramInfoLog(g_Program, 1024, NULL, errorLog);
		LOG(INFO) << errorLog << std::endl;
    }
    
    glGetProgramiv( g_Program, GL_ATTACHED_SHADERS, &result );
    LOG(INFO) << "Attached shaders status: " << result << std::endl;
    
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

static void SetDefaultGraphicsState ();
static void DoRendering (const glm::mat4& worldMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& projectionMatrix );

LODPlane* lodPlane;

void EXPORT_API InitPlugin()
{
    lodPlane = new LODPlane;
}

void EXPORT_API DestroyPlugin()
{
    delete lodPlane;
}


void EXPORT_API UnityRenderEvent (int eventID)
{
	// Unknown graphics device type? Do nothing.
	if (g_DeviceType == -1)
		return;

	// Actual functions defined below
	SetDefaultGraphicsState ();
	DoRendering( modelMatrix_, viewMatrix_, projectionMatrix_ );
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
                         const glm::mat4& projectionMatrix )
{
        // Set shader program
        glUseProgram(g_Program);
        
        // Send modelview matrix to shader.
        const glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glUniformMatrix4fv(g_WorldMatrixUniformIndex, 1, GL_FALSE, glm::value_ptr( modelViewMatrix ) );
        
        // Send projection matrix to shader.
        glUniformMatrix4fv(g_ProjMatrixUniformIndex, 1, GL_FALSE, glm::value_ptr( projectionMatrix ) );
    
        // Compute the distance between the camera and the plane.
        // TODO: Compute real distance.
        const float distance = glm::length( cameraPos_ );
    
        // Render the plane
        lodPlane->render( distance );

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
