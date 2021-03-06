// Example low level rendering Unity plugin

#include <RenderingPlugin.h>

#include <math.h>
#include <stdio.h>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <lod_plane.hpp>
#include <shaders.hpp>

// --------------------------------------------------------------------------
// Helper utilities

std::unique_ptr<LODPlane> lodPlane;

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


void EXPORT_API SetPlaneTextureFromUnity( GLuint texturePtr, unsigned int lodLevel )
{
    lodPlane->setTextureID( texturePtr, lodLevel );
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
    const char logFilePath[] = "rendering-plugin-log.txt";
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.set(el::Level::Info, el::ConfigurationType::Filename, logFilePath );
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

    InitShaders();
    
    g_DeviceType = deviceType;

    el::Loggers::flushAll();
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

void EXPORT_API InitPlugin()
{
    lodPlane = std::unique_ptr<LODPlane>( new LODPlane );
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


static void SetDefaultGraphicsState ()
{
	glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
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
    if( UsePluginShader() ){
        // Send modelview matrix to shader.
        SendMatricesToShader( modelMatrix, viewMatrix, projectionMatrix );
    
        // Compute the distance between the camera and the plane.
        const float distance = glm::distance( cameraPos_, lodPlane->centroid() );
    
        // Render the plane
        lodPlane->render( distance );
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
