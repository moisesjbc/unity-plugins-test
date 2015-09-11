#ifndef RENDERING_PLUGIN_H
#define RENDERING_PLUGIN_H

#include <platform.hpp>

// Graphics device identifiers in Unity
enum GfxDeviceRenderer
{
	kGfxRendererOpenGL = 0,			// OpenGL
	kGfxRendererD3D9,				// Direct3D 9
	kGfxRendererD3D11,				// Direct3D 11
	kGfxRendererGCM,				// Sony PlayStation 3 GCM
	kGfxRendererNull,				// "null" device (used in batch mode)
	kGfxRendererHollywood,			// Nintendo Wii
	kGfxRendererXenon,				// Xbox 360
	kGfxRendererOpenGLES_Obsolete,
	kGfxRendererOpenGLES20Mobile,	// OpenGL ES 2.0
	kGfxRendererMolehill_Obsolete,
	kGfxRendererOpenGLES20Desktop_Obsolete,
	kGfxRendererOpenGLES30,			// OpenGL ES 3.0
	kGfxRendererCount
};


// Event types for UnitySetGraphicsDevice
enum GfxDeviceEventType {
	kGfxDeviceEventInitialize = 0,
	kGfxDeviceEventShutdown,
	kGfxDeviceEventBeforeReset,
	kGfxDeviceEventAfterReset,
};


extern "C"
{
    void EXPORT_API InitPlugin();
    void EXPORT_API SetTimeFromUnity (float t);
    void EXPORT_API SetMatricesFromUnity( float* modelMatrix,
                                            float* viewMatrix,
                                            float* projectionMatrix );
    void EXPORT_API SetTextureFromUnity(void* texturePtr, int w, int h);
    void EXPORT_API UnitySetGraphicsDevice ( void* device, int deviceType, int eventType );
    void EXPORT_API UnityRenderEvent (int eventID);
}

#endif // RENDERING_PLUGIN_H
