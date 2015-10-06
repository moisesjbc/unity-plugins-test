#ifndef RENDERING_PLUGIN_H
#define RENDERING_PLUGIN_H

#include <platform.hpp>

extern "C"
{
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTimeFromUnity (float t);
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetMatricesFromUnity( float* modelMatrix,
                                            float* viewMatrix,
                                            float* projectionMatrix );
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTextureFromUnity(void* texturePtr, int w, int h);
    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetPlaneTextureFromUnity( GLuint texturePtr, unsigned int lodLevel );

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetUnityStreamingAssetsPath(const char* path);
}

#endif // RENDERING_PLUGIN_H
