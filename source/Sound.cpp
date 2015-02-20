#include "Sound.hpp"
#include "Utils.hpp"

using namespace bryte;

Bool Sound::load_effects ( )
{
     for ( Uint32 i = 0; i < Effect::count; ++i ) {
          m_effects [ i ] = nullptr;
     }

     if ( !load_effect ( Effect::player_sword_attack, "player_sword_attack.wav" ) ) {
          return false;
     }

     if ( !load_effect ( Effect::player_damaged, "player_damaged.wav" ) ) {
          return false;
     }

     if ( !load_effect ( Effect::player_pickup, "player_pickup.wav" ) ) {
          return false;
     }

     if ( !load_effect ( Effect::place_bomb, "place_bomb.wav" ) ) {
          return false;
     }

     if ( !load_effect ( Effect::enemy_damaged, "enemy_damaged.wav" ) ) {
          return false;
     }

     if ( !load_effect ( Effect::bomb_exploded, "bomb_exploded.wav" ) ) {
          return false;
     }

     if ( !load_effect ( Effect::activate_interactive, "activate_interactive.wav" ) ) {
          return false;
     }

     return true;
}

Void Sound::unload_effects ( )
{
     for ( Uint32 i = 0; i < Effect::count; ++i ) {
          Mix_FreeChunk ( m_effects [ i ] );
          m_effects [ i ] = nullptr;
     }
}

Bool Sound::play_effect ( Effect effect )
{
     ASSERT ( effect < Effect::count );

     if ( Mix_PlayChannel ( -1, m_effects [ effect ], 0 ) < 0 ) {
          LOG_ERROR ( "Mix_PlayChannel() failed: %s\n", Mix_GetError ( ) );
          return false;
     }

     return true;
}

Bool Sound::load_effect ( Effect effect, const char* wav_filepath )
{
     m_effects [ effect ] = Mix_LoadWAV ( wav_filepath );

     if ( !m_effects [ effect ] ) {
          LOG_ERROR ( "Mix_LoadWAV(%s) failed: %s\n", wav_filepath, Mix_GetError ( ) );
          return false;
     }

     return true;
}

