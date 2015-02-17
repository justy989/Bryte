#ifndef BRYTE_SOUND_EFFECT_HPPP
#define BRYTE_SOUND_EFFECT_HPPP

#include <SDL2/SDL_mixer.h>

#include "Types.hpp"

namespace bryte
{
     struct Sound {
     public:

          enum Effect {
               player_attack,
               count
          };

          Bool load_effects ( );
          Void unload_effects ( );

          Bool play_effect ( Effect effect );

     private:

          Bool load_effect ( Effect effect, const char* wav_filepath );

     private:

          Mix_Chunk* m_effects [ Effect::count ];
     };
}

#endif

