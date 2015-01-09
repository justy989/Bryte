#include "Utils.hpp"
#include "Globals.hpp"

#include <fstream>

using namespace bryte;

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

Void FileContents::free ( )
{
     if ( size && bytes ) {
          Globals::g_game_memory.pop_array<Char8>( size );
     }

     size  = 0;
     bytes = nullptr;
}

extern "C" FileContents load_entire_file ( const Char8* filepath )
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

     contents.bytes = Globals::g_game_memory.push_array<Char8>( contents.size );

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
