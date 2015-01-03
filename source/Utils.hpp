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
     static const Real32 pixels_per_meter = 10.0f;

     inline Int32 meters_to_pixels ( Real32 meters )
     {
          return static_cast<Int32>( meters * pixels_per_meter );
     }

     inline Real32 pixels_to_meters ( Int32 pixels )
     {
          return static_cast<Real32>( pixels ) / pixels_per_meter;
     }

     inline Void convert_to_sdl_origin_for_surface ( SDL_Rect& rect, SDL_Surface* surface )
     {
          rect.y = ( surface->h - rect.y ) - rect.h;
     }
}

#endif

