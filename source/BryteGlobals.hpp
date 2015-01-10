#ifndef BRYTE_GLOBALS_HPP
#define BRYTE_GLOBALS_HPP

#include "GameMemory.hpp"
#include "MemoryLocations.hpp"

namespace bryte
{
     class Globals {
     public:

          static GameMemory      g_game_memory;
          static MemoryLocations g_memory_locations;
     };
}

#endif

