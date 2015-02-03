#include "Utils.hpp"
#include "GameMemory.hpp"
#include "Vector.hpp"
#include "Random.hpp"

#include <fstream>

extern "C" Bitscan bitscan_forward ( Uint32 mask )
{
     static const Uint32 bit_count = sizeof ( mask ) * BITS_PER_BYTE;
     Bitscan result;

     for ( Uint32 i = 0; i < bit_count; ++i ) {
          if ( ( 1 << i ) & mask ) {
               result.found = true;
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

     FileContents contents;

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open file '%s' to load entire file\n", filepath );
          return contents;
     }

     file.seekg ( 0, file.end );
     contents.size = file.tellg ( );
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

