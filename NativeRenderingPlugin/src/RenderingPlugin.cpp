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
static glm::mat4 modelMatrix_;
static glm::mat4 viewMatrix_;
static glm::mat4 projectionMatrix_;
static glm::vec4 cameraPos_;

// Prints a string
static void DebugLog (const char* str)
{
	#if UNITY_WIN
	OutputDebugStringA (str);
	#else
	printf ("%s", str);
	#endif
}


void LogOpenGLVersion()
{
    const GLubyte* oglVersion = glGetString( GL_VERSION );

    if( oglVersion ){
        LOG(INFO) << "oglVersion: " << oglVersion << std::endl;
    }
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

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTimeFromUnity (float t)
{
    g_Time = t;
}


// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

static void*	g_TexturePointer	= 0;
static int		g_TexWidth			= 0;
static int		g_TexHeight			= 0;

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTextureFromUnity(void* texturePtr, int w, int h)
{
    g_TexturePointer	= texturePtr;
    g_TexWidth			= w;
    g_TexHeight			= h;
}


// --------------------------------------------------------------------------
// SetUnityStreamingAssetsPath, an example function we export which is called by one of the scripts.

static std::string s_UnityStreamingAssetsPath;
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetUnityStreamingAssetsPath(const char* path)
{
    s_UnityStreamingAssetsPath = path;
}


// --------------------------------------------------------------------------
// UnitySetInterfaces

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    // Configure logger
    #if !__ANDROID__
        const char logFilePath[] = "rendering-plugin-log.txt";
        el::Configurations defaultConf;
        defaultConf.setToDefault();
        defaultConf.set(el::Level::Info, el::ConfigurationType::Filename, logFilePath );
        el::Loggers::reconfigureLogger("default", defaultConf);
    #endif

    LOG(INFO) << "UnityPluginLoad() - 1" << std::endl;

    s_UnityInterfaces = unityInterfaces;
    s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
    s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

    LOG(INFO) << "UnityPluginLoad() - 2" << std::endl;

    // Run OnGraphicsDeviceEvent(initialize) manually on plugin load
    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
    s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}


// --------------------------------------------------------------------------
// GraphicsDeviceEvent

// Actual setup/teardown functions defined below
static void DoEventGraphicsDeviceGLES(UnityGfxDeviceEventType eventType);

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    UnityGfxRenderer currentDeviceType = s_DeviceType;

    switch (eventType)
    {
    case kUnityGfxDeviceEventInitialize:
        {
            LOG(INFO) << "OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize)" << std::endl;
            DebugLog("OnGraphicsDeviceEvent(Initialize).\n");
            s_DeviceType = s_Graphics->GetRenderer();
            currentDeviceType = s_DeviceType;
            break;
        }

    case kUnityGfxDeviceEventShutdown:
        {
            DebugLog("OnGraphicsDeviceEvent(Shutdown).\n");
            s_DeviceType = kUnityGfxRendererNull;
            g_TexturePointer = NULL;
            break;
        }

    case kUnityGfxDeviceEventBeforeReset:
        {
            DebugLog("OnGraphicsDeviceEvent(BeforeReset).\n");
            break;
        }

    case kUnityGfxDeviceEventAfterReset:
        {
            DebugLog("OnGraphicsDeviceEvent(AfterReset).\n");
            break;
        }
    };

    if (currentDeviceType == kUnityGfxRendererOpenGLES20 ||
        currentDeviceType == kUnityGfxRendererOpenGLES30 ||
        currentDeviceType == kUnityGfxRendererOpenGL )
        DoEventGraphicsDeviceGLES(eventType);
}


// --------------------------------------------------------------------------
// OnRenderEvent
// This will be called for GL.IssuePluginEvent script calls; eventID will
// be the integer passed to IssuePluginEvent. In this example, we just ignore
// that value.

static void SetDefaultGraphicsState ();
static void DoRendering ( const glm::mat4& modelMatrix,
                         const glm::mat4& viewMatrix,
                         const glm::mat4& projectionMatrix );

static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
    // Unknown graphics device type? Do nothing.
    if (s_DeviceType == kUnityGfxRendererNull)
        return;

    // Actual functions defined below
    SetDefaultGraphicsState ();
    DoRendering (modelMatrix_, viewMatrix_, projectionMatrix_);
}

// --------------------------------------------------------------------------
// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
    return OnRenderEvent;
}


static void DoEventGraphicsDeviceGLES(UnityGfxDeviceEventType eventType)
{
    if (eventType == kUnityGfxDeviceEventInitialize)
    {
        if ((s_DeviceType != kUnityGfxRendererOpenGL) && (s_DeviceType != kUnityGfxRendererOpenGLES20)){
            LOG(ERROR) << "NO OPENGL (" << s_DeviceType << ")" << std::endl;
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

        lodPlane = std::unique_ptr<LODPlane>( new LODPlane );

        el::Loggers::flushAll();
    }else if (eventType == kUnityGfxDeviceEventShutdown){

    }
}


static void SetDefaultGraphicsState ()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    //glDisable(GL_BLEND);
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


/*********************************************************************/


void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetMatricesFromUnity( float* modelMatrix,
                                        float* viewMatrix,
                                        float* projectionMatrix )
{
    modelMatrix_ = glm::make_mat4( modelMatrix );
    viewMatrix_ = glm::make_mat4( viewMatrix );
    projectionMatrix_ = glm::make_mat4( projectionMatrix );
    cameraPos_ = glm::inverse( viewMatrix_ ) * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
}


void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetPlaneTextureFromUnity( GLuint texturePtr, unsigned int lodLevel )
{
    lodPlane->setTextureID( texturePtr, lodLevel );
}
