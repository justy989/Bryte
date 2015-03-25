#ifndef BRYTE_UTILS_HPP
#define BRYTE_UTILS_HPP

#include "Log.hpp"
#include "Types.hpp"
#include "Direction.hpp"

#include <SDL2/SDL.h>
#include <cassert>

#define BITS_PER_BYTE 8
#define KILOBYTES( bytes ) bytes * 1024
#define MEGABYTES( bytes ) KILOBYTES( bytes ) * 1024
#define GIGABYTES( bytes ) MEGABYTES( bytes ) * 1024

#ifdef DEBUG
     #define ASSERT( condition )\
     if ( !( condition ) ) {\
          LOG_ERROR ( "Assert failed %s:%d ( %s )\n", __FILE__, __LINE__, #condition );\
          assert ( condition );\
     }
#else
     #define ASSERT( condition )
#endif

#define CLAMP( value, min, max ) if ( value < min ) { value = min; } else if ( value > max ) { value = max; }

#define FREE_SURFACE( surface ) if ( surface ) { SDL_FreeSurface ( surface ); surface = nullptr; }

inline Real32 square ( Real32 value )
{
     return value * value;
}

// forward declaration for load_entire_file
class GameMemory;

struct Bitscan {
     Int32 found;
     Uint32 bit;
};

extern "C" Bitscan bitscan_forward ( Uint32 mask );

struct FileContents {
     Char8* bytes;
     Uint32 size;

     Void free ( GameMemory* game_memory );
};

// allocates bytes using game memory
extern "C" FileContents load_entire_file ( const Char8* filepath, GameMemory* game_memory );

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

inline Void world_to_sdl ( SDL_Rect& rect, SDL_Surface* back_buffer, Real32 camera_x, Real32 camera_y )
{
     // offset by the camera
     rect.x += meters_to_pixels ( camera_x );
     rect.y += meters_to_pixels ( camera_y );

     // recalc the y so the bottom left is the origin
     rect.y = ( back_buffer->h - rect.y ) - rect.h;
}

inline SDL_Rect build_world_sdl_rect ( Real32 x, Real32 y, Real32 w, Real32 h )
{
     SDL_Rect rect = { meters_to_pixels ( x ), meters_to_pixels ( y ),
                       meters_to_pixels ( w ), meters_to_pixels ( h ) };

     return rect;
}

inline Bool point_inside_rect ( Real32 x, Real32 y, Real32 l, Real32 b, Real32 r, Real32 t )
{
     return ( x >= l && x <= r && y >= b && y <= t );
}

Bool rect_collides_with_rect ( Real32 a_left, Real32 a_bottom, Real32 a_width, Real32 a_height,
                               Real32 b_left, Real32 b_bottom, Real32 b_width, Real32 b_height );

namespace bryte {
     class Random;
};

class Vector;

bryte::Direction direction_between ( const Vector& a, const Vector& b, bryte::Random& random );

Void render_rect_outline ( SDL_Surface* dest_surface, const SDL_Rect& rect, Uint32 color );

Vector vector_from_direction ( bryte::Direction dir );

Void move_location ( Int32& tile_x, Int32& tile_y, bryte::Direction move_dir );

#endif

