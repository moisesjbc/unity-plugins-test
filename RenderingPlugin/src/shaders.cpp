#include <shaders.hpp>

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


void InitShaders()
{
    char vertexShaderCode[] =
        "attribute vec3 pos;\
        attribute vec4 color;\
        attribute vec2 uv;\
        \
        varying vec4 ocolor;\
        varying vec2 ouv;\
        \
        uniform mat4 worldMatrix;\
        uniform mat4 projMatrix;\
        \
        void main()\
        {\
            gl_Position = (projMatrix * worldMatrix) * vec4(pos,1);\
            ocolor = color;\
            ouv = uv;\
        }";

    char fragmetShaderCode[] =
        "precision mediump float;\
        varying vec4 ocolor;\
        varying vec2 ouv;\
        \
        uniform sampler2D textureSampler;\
        \
        void main()\
        {\
            gl_FragColor = texture2D( textureSampler, ouv );\
        }";

    g_VProg		= CreateShader(GL_VERTEX_SHADER, vertexShaderCode);
    g_FShader	= CreateShader(GL_FRAGMENT_SHADER, fragmetShaderCode);

    checkOpenGLStatus( "UnitySetGraphicsDevice - 2" );

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
}


void SendMatricesToShader( const glm::mat4& modelMatrix,
                           const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix )
{
    const glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    glUniformMatrix4fv(g_WorldMatrixUniformIndex, 1, GL_FALSE, glm::value_ptr( modelViewMatrix ) );

    // Send projection matrix to shader.
    glUniformMatrix4fv(g_ProjMatrixUniformIndex, 1, GL_FALSE, glm::value_ptr( projectionMatrix ) );
}


bool UsePluginShader()
{
    if( g_Program != 0 ){
        // Set shader program
        glUseProgram(g_Program);
        return true;
    }else{
        return false;
    }
}
