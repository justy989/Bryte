#include "Utils.hpp"
#include "GameMemory.hpp"
#include "Vector.hpp"
#include "Random.hpp"

#include <fstream>

extern "C" Bitscan bitscan_forward ( Uint32 mask )
{
     static const Uint32 bit_count = sizeof ( mask ) * BITS_PER_BYTE;
     Bitscan result { 0, sizeof ( Uint32 ) * BITS_PER_BYTE + 1 };

     for ( Uint32 i = 0; i < bit_count; ++i ) {
          if ( ( 1 << i ) & mask ) {
               result.found = 1;
               result.bit = i;
               break;
          }
     }

     return result;
}

Void FileContents::free ( GameMemory* game_memory )
{
     if ( size && bytes ) {
          GAME_PUSH_MEMORY_ARRAY ( (*game_memory), Char8, size );
     }

     size  = 0;
     bytes = nullptr;
}

extern "C" FileContents load_entire_file ( const Char8* filepath, GameMemory* game_memory )
{
     LOG_DEBUG ( "Loading entire file '%s'\n", filepath );
     std::ifstream file ( filepath, std::ios::binary );

     FileContents contents { nullptr, 0 };

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open file '%s' to load entire file\n", filepath );
          return contents;
     }

     file.seekg ( 0, file.end );
     contents.size = static_cast<Uint32>( file.tellg ( ) );
     file.seekg ( 0, file.beg );

     contents.bytes = GAME_PUSH_MEMORY_ARRAY ( (*game_memory), Char8, contents.size );

     if ( !contents.bytes ) {
          LOG_ERROR ( "Failed to allocate memory to read file '%s' into of size %d bytes\n",
                      filepath, contents.size );
          contents.size = 0;
          return contents;
     }

     file.read ( contents.bytes, contents.size );
     file.close ( );

     return contents;
}

bryte::Direction direction_between ( const Vector& a, const Vector& b, bryte::Random& random )
{
     Vector diff = b - a;

     Real32 abs_x = fabs ( diff.x ( ) );
     Real32 abs_y = fabs ( diff.y ( ) );

     if ( abs_x > abs_y ) {
          if ( diff.x ( ) > 0.0f ) {
               return bryte::Direction::right;
          }

          return bryte::Direction::left;
     } else if ( abs_y > abs_x ) {
          if ( diff.y ( ) > 0.0f ) {
               return bryte::Direction::up;
          }

          return bryte::Direction::down;
     } else {
          bryte::Direction valid_dirs [ 2 ];

          if ( diff.x ( ) > 0.0f ) {
               valid_dirs [ 0 ] = bryte::Direction::right;
          } else {
               valid_dirs [ 0 ] = bryte::Direction::left;
          }

          if ( diff.y ( ) > 0.0f ) {
               valid_dirs [ 1 ] = bryte::Direction::up;
          } else {
               valid_dirs [ 1 ] = bryte::Direction::down;
          }

          // coin flip between using the x or y direction
          return valid_dirs [ random.generate ( 0, 2 ) ];
     }

     // the above cases should catch all
     ASSERT ( 0 );
     return bryte::Direction::left;
}

Bool rect_collides_with_rect ( Real32 a_left, Real32 a_bottom, Real32 a_width, Real32 a_height,
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

Void render_rect_outline ( SDL_Surface* dest_surface, const SDL_Rect& rect, Uint32 color )
{
     SDL_Rect left_line { rect.x, rect.y, 1, rect.h };
     SDL_Rect right_line { rect.x + ( rect.w - 1 ), rect.y, 1, rect.h };
     SDL_Rect bottom_line { rect.x, rect.y, rect.w, 1 };
     SDL_Rect top_line { rect.x, rect.y + ( rect.h - 1 ), rect.w, 1 };

     SDL_FillRect ( dest_surface, &left_line, color );
     SDL_FillRect ( dest_surface, &right_line, color );
     SDL_FillRect ( dest_surface, &bottom_line, color );
     SDL_FillRect ( dest_surface, &top_line, color );
}

Vector vector_from_direction ( bryte::Direction dir )
{
     switch ( dir )
     {
     default:
          ASSERT ( 0 );
          break;
     case bryte::Direction::left:
          return Vector { -1.0f, 0.0f };
     case bryte::Direction::up:
          return Vector { 0.0f, 1.0f };
     case bryte::Direction::right:
          return Vector { 1.0f, 0.0f };
     case bryte::Direction::down:
          return Vector { 0.0f, -1.0f };
     }

     // should not hit
     return Vector { 0.0f, 0.0f };
}


