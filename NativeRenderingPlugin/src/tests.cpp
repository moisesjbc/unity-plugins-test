#include <iostream>
#include <SDL2/SDL.h>
#include <stdexcept>
#include <GLES2/gl2.h>

int RES_X = 400;
int RES_Y = 300;

int main( int argc, char* argv[] )
{
    // Initialize the SDL library
    if( SDL_Init( SDL_INIT_VIDEO ) ){
        throw std::runtime_error( SDL_GetError() );
    }
    atexit( SDL_Quit );

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window_ = SDL_CreateWindow( "RenderingPlugin",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                RES_X,
                                RES_Y,
                                SDL_WINDOW_OPENGL );

    if( !window_ ){
        throw std::runtime_error( SDL_GetError() );
    }

    SDL_GLContext glcontext = SDL_GL_CreateContext( window_ );

    //SDL_Renderer* renderer = SDL_CreateRenderer( window_, -1, 0);
    //if( !renderer ){
    //    throw std::runtime_error( SDL_GetError() );
    //}

    SDL_Event event;
    bool exitTest = false;
    while( !exitTest ){
        while( SDL_PollEvent( &event ) ){
            exitTest = ( event.type == SDL_QUIT );
        }
        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT );
        SDL_GL_SwapWindow( window_ );
    }


    SDL_GL_DeleteContext( glcontext );
    return 0;
}
