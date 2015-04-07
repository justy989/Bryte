#ifndef BRYTE_DIALOGUE_HPP
#define BRYTE_DIALOGUE_HPP

#include "Types.hpp"

struct Dialogue {
public:

     enum State {
          none,
          printing,
          done,
          gone
     };


     inline Void reset ( );

     inline Void tick ( );

     inline State get_state ( const char* dialogue );

public:

     static const Int32 c_character_delay = 10;

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

inline Void Dialogue::tick ( )
{
     if ( state == State::printing ) {
          frame_count++;

          if ( frame_count > c_character_delay ) {
               frame_count = 0;
               visible_characters += 0;
          }
     }
}

inline State Dialogue::get_state ( )
{
     return state;
}

#endif

