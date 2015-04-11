#ifndef BRYTE_DIALOGUE_HPP
#define BRYTE_DIALOGUE_HPP

#include <SDL2/SDL.h>

#include "Utils.hpp"
#include "Types.hpp"
#include "Text.hpp"

struct Dialogue {
public:

     enum State {
          none,
          printing,
          done,
     };

public:

     inline Void reset ( );

     inline Void begin ( );
     inline Void force_done ( const char* dialogue );

     inline Void tick ( const char* dialogue );

     Void render ( SDL_Surface* back_buffer, Text* text,
                   const Char8* message );

     inline State get_state ( );
     inline Int32 get_visible_characters ( );

public:

     static const Int32 c_character_delay = 4;
     static const Int32 c_dialogue_height = 200;

public:

     Int32 visible_characters;
     Int32 frame_count;

     State state;
};

inline Void Dialogue::reset ( )
{
     state = State::none;
     visible_characters = 0;
     frame_count = 0;
}

inline Void Dialogue::begin ( )
{
     ASSERT ( state == State::none );

     state = State::printing;
}

inline Void Dialogue::force_done ( const char* dialogue )
{
     visible_characters = strlen ( dialogue );

     state = State::done;
}

inline Void Dialogue::tick ( const char* dialogue )
{
     if ( state == State::printing ) {
          frame_count++;

          if ( frame_count > c_character_delay ) {
               frame_count = 0;
               visible_characters++;

               if ( visible_characters == static_cast<Int32>( strlen ( dialogue ) ) ) {
                    state = State::done;
               }
          }
     }
}

inline Dialogue::State Dialogue::get_state ( )
{
     return state;
}

inline Int32 Dialogue::get_visible_characters ( )
{
     return visible_characters;
}

#endif

