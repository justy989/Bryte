#ifndef BRYTE_UTILS_HPP
#define BRYTE_UTILS_HPP

#include "Log.hpp"
#include "Types.hpp"

#include <SDL2/SDL.h>

#define KILOBYTES( bytes ) bytes * 1024
#define MEGABYTES( bytes ) KILOBYTES( bytes ) * 1024
#define GIGABYTES( bytes ) MEGABYTES( bytes ) * 1024

#ifdef DEBUG
     #define ASSERT( condition )\
     if ( !( condition ) ) {\
          LOG_ERROR ( "Assert failed %s:%d ( %s )\n", __FILE__, __LINE__, #condition );\
          *reinterpret_cast<char*>(0) = 0;\
     }
#else
     #define ASSERT( condition )
#endif

#define CLAMP( value, min, max ) if ( value < min ) { value = min; } else if ( value > max ) { value = max; }

namespace bryte
{
     // does not work for negative reals
     inline Int32 positive_real_to_int_round ( Real32 value )
     {
          return static_cast<Int32>( value + 1.5f );
     }

     static const Real32 pixels_per_meter = 10.0f;

     inline Int32 meters_to_pixels ( Real32 meters )
     {
          return positive_real_to_int_round ( meters * pixels_per_meter );
     }

     inline Real32 pixels_to_meters ( Int32 pixels )
     {
          return static_cast<Real32>( pixels ) / pixels_per_meter;
     }

     inline Void convert_to_sdl_origin_for_surface ( SDL_Rect& rect, SDL_Surface* surface )
     {
          rect.y = ( surface->h - rect.y ) - rect.h;
     }

     inline Bool point_inside_rect ( Real32 x, Real32 y, Real32 l, Real32 b, Real32 r, Real32 t )
     {
          return ( x >= l && x <= r && y >= b && y <= t );
     }

     inline Bool rect_collides_with_rect ( Real32 a_left, Real32 a_bottom, Real32 a_width, Real32 a_height,
                                           Real32 b_left, Real32 b_bottom, Real32 b_width, Real32 b_height )
     {
          Real32 b_right = b_left + b_width;
          Real32 b_top = b_bottom + b_height;

          // test A inside B
          if ( point_inside_rect ( a_left, a_bottom,
                                   b_left, b_bottom, b_right, b_top ) ||
               point_inside_rect ( a_left + a_width, a_bottom,
                                   b_left, b_bottom, b_right, b_top ) ||
               point_inside_rect ( a_left, a_bottom + a_height,
                                   b_left, b_bottom, b_right, b_top ) ||
               point_inside_rect ( a_left + a_width, a_bottom + a_height,
                                   b_left, b_bottom, b_right, b_top ) ) {
               return true;
          }

          // test A inside B center
          if ( point_inside_rect ( a_left + a_width * 0.5f, a_bottom + a_height * 0.5f,
                                   b_left, b_bottom, b_right, b_top ) ) {
               return true;
          }

          Real32 a_right = a_left + a_width;
          Real32 a_top = a_bottom + a_height;

          // test B inside A
          if ( point_inside_rect ( b_left, b_bottom,
                                   a_left, a_bottom, a_right, a_top ) ||
               point_inside_rect ( b_left + b_width, b_bottom,
                                   a_left, a_bottom, a_right, a_top ) ||
               point_inside_rect ( b_left, b_bottom + b_height,
                                   a_left, a_bottom, a_right, a_top ) ||
               point_inside_rect ( b_left + b_width, b_bottom + b_height,
                                   a_left, a_bottom, a_right, a_top ) ) {
               return true;
          }

          return false;
     }
}

#endif

