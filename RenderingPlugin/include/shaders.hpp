#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <platform.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

static GLuint CreateShader(GLenum type, const char* text );
void InitShaders();
void SendMatricesToShader( const glm::mat4& modelMatrix,
                           const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix );
bool UsePluginShader();

#endif // SHADERS_HPP

