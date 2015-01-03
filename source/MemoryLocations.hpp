#ifndef BRYTE_MEMORY_LOCATIONS_HPP
#define BRYTE_MEMORY_LOCATIONS_HPP

#include "Bryte.hpp"
#include "Map.hpp"

namespace bryte
{
     struct MemoryLocations {
     public:

          inline MemoryLocations ( );

     public:

          GameState* game_state;

          Map::Room* rooms;
     };

     MemoryLocations::MemoryLocations ( ) :
          game_state ( nullptr ),
          rooms ( nullptr )
     {

     }
}

#endif

