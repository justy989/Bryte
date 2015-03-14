#ifndef BRYTE_PLAYER_HPP
#define BRYTE_PLAYER_HPP

#include "Character.hpp"

namespace bryte
{
     struct Player : public Character {
     public:

          enum ItemMode {
               shield,
               arrow,
               bomb,
               count
          };

          Void clear ( );

          Bool save ( );
          Bool load ( );

     public:

          static const Real32 c_item_cooldown;

     public:

          ItemMode item_mode;
          Stopwatch item_cooldown;

          Uint8 key_count;
          Uint8 arrow_count;
          Uint8 bomb_count;

          Uint8 save_slot;
     };
}

#endif

