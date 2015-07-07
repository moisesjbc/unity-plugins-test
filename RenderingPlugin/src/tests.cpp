#include <iostream>
#include <SDL2/SDL.h>
#include <stdexcept>

int RES_X = 400;
int RES_Y = 300;

int main( int argc, char* argv[] )
{
    // Initialize the SDL library
    if( SDL_Init( SDL_INIT_VIDEO ) ){
        throw std::runtime_error( SDL_GetError() );
    }
    atexit( SDL_Quit );

    SDL_Window* window_ = SDL_CreateWindow( "RenderingPlugin",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                RES_X,
                                RES_Y,
                                0 );
    if( !window_ ){
        throw std::runtime_error( SDL_GetError() );
    }

    SDL_Renderer* renderer = SDL_CreateRenderer( window_, -1, 0);
    if( !renderer ){
        throw std::runtime_error( SDL_GetError() );
    }

    SDL_Event event;
    bool exitTest = false;
    while( !exitTest ){
        while( SDL_PollEvent( &event ) ){
            exitTest = ( event.type == SDL_QUIT );
        }
        SDL_RenderClear( renderer );
        SDL_RenderPresent( renderer );
    }
    return 0;
}
