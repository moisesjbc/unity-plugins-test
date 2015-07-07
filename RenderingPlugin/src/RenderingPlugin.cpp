// Example low level rendering Unity plugin


#include "UnityPluginInterface.h"

#include <math.h>
#include <stdio.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>

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



// --------------------------------------------------------------------------
// SetTimeFromUnity, an example function we export which is called by one of the scripts.

static float g_Time;

extern "C" void EXPORT_API SetTimeFromUnity (float t) { g_Time = t; }

static glm::mat4 modelMatrix_;
static glm::mat4 viewMatrix_;
static glm::mat4 projectionMatrix_;
static glm::vec4 cameraPos_;

extern "C" void EXPORT_API SetMatricesFromUnity( float* modelMatrix,
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

static void* g_TexturePointer;

extern "C" void EXPORT_API SetTextureFromUnity (void* texturePtr)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).
	g_TexturePointer = texturePtr;
}



// --------------------------------------------------------------------------
// UnitySetGraphicsDevice

static int g_DeviceType = -1;

extern "C" void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
	// If we've got an OpenGL device, remember device type. There's no OpenGL
	// "device pointer" to remember since OpenGL always operates on a currently set
	// global context.
	if (deviceType == kGfxRendererOpenGL)
	{
		DebugLog ("Set OpenGL graphics device\n");
		g_DeviceType = deviceType;
	}
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

extern "C" void EXPORT_API InitPlugin()
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


extern "C" void EXPORT_API UnityRenderEvent (int eventID)
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
	if (g_DeviceType == kGfxRendererOpenGL)
	{
		glDisable (GL_CULL_FACE);
		glDisable (GL_LIGHTING);
		glDisable (GL_BLEND);
		glDisable (GL_ALPHA_TEST);
		glDepthFunc (GL_LEQUAL);
		glEnable (GL_DEPTH_TEST);
		glDepthMask (GL_FALSE);
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}
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
	// OpenGL case
	if (g_DeviceType == kGfxRendererOpenGL)
	{
		// Transformation matrices
		glMatrixMode (GL_MODELVIEW);
        const glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glLoadMatrixf ( glm::value_ptr( modelViewMatrix ) );
		glMatrixMode (GL_PROJECTION);
        glLoadMatrixf ( glm::value_ptr( projectionMatrix ) );

		// Vertex layout
		glVertexPointer (3, GL_FLOAT, sizeof(verts[0]), &verts[0].x);
		glEnableClientState (GL_VERTEX_ARRAY);
		glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(verts[0]), &verts[0].color);
		glEnableClientState (GL_COLOR_ARRAY);

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
			glBindTexture (GL_TEXTURE_2D, gltex);
			int texWidth, texHeight;
			glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
			glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

			unsigned char* data = new unsigned char[texWidth*texHeight*4];
			FillTextureFromCode (texWidth, texHeight, texHeight*4, data);
			glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
			delete[] data;
		}
	}
}
