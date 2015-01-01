#include "draw_utils.hpp"

#include <cassert>
#include <stdexcept>

using namespace bryte;

void draw_utils::draw_border ( const rectangle& outline, Uint32 color,
                               SDL_Surface* back_buffer )
{
     // top bar
     SDL_Rect rect { outline.left ( ) - 1,
                     outline.bottom ( ) - 1,
                     outline.width ( ) + 2,
                     1 };
     SDL_FillRect ( back_buffer, &rect, color );

     // bottom bar
     rect = SDL_Rect { outline.left ( ) - 1,
                       outline.top ( ),
                       outline.width ( ) + 2,
                       1 };
     SDL_FillRect ( back_buffer, &rect, color );

     // left bar
     rect = SDL_Rect { outline.left ( ) - 1,
                       outline.bottom ( ) - 1,
                       1,
                       outline.height ( ) + 2 };
     SDL_FillRect ( back_buffer, &rect, color );

     // right bar
     rect = SDL_Rect { outline.right ( ),
                       outline.bottom ( ) - 1,
                       1,
                       outline.height ( ) + 2 };
     SDL_FillRect ( back_buffer, &rect, color );
}

void draw_utils::rotate_square_clockwise ( SDL_Surface* surface,
                                           const rectangle& square )
{
     assert ( square.width ( ) == square.height ( ) );
     assert ( surface->format->format == SDL_PIXELFORMAT_RGBA8888 );

     // lock the surface
     if ( SDL_LockSurface ( surface ) ) {
          throw std::runtime_error ( std::string( "SDL_LockSurface(): " ) +
                                     SDL_GetError ( ) );
     }

     int n = square.width ( );
     int n_squared = n * n;

     Uint32* normal_pixels = reinterpret_cast<Uint32*>(surface->pixels);
     Uint32* rotated_pixels = new Uint32 [ n_squared ];

     // rotate the pixels into the new buffer
     for ( int y = 0; y < n; ++y ) {
          for ( int x = 0; x < n; ++x ) {
               int rotated_index = (n - 1 - x) * n + y; // [ n - 1 - x ][y]
               int normal_index = ( square.bottom ( ) + y ) * surface->w + square.left ( ) + x;   // [y][x]

               rotated_pixels [ rotated_index ] = normal_pixels [ normal_index ];
          }
     }

     // copy back out to the surface
     for ( int y = 0; y < n; ++y ) {
          for ( int x = 0; x < n; ++x ) {
               int rotated_index = y * n + x;
               int normal_index = ( square.bottom ( ) + y ) * surface->w + square.left ( ) + x;

               normal_pixels [ normal_index ] = rotated_pixels [ rotated_index ];
          }
     }

     // unlock the surface
     SDL_UnlockSurface ( surface );

     delete[] rotated_pixels;
}

