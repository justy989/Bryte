#ifndef BRYTE_PLAYER_HPP
#define BRYTE_PLAYER_HPP

#include "Character.hpp"

namespace bryte
{
     struct Player : public Character {

          enum AttackMode {
               sword,
               arrow,
               bomb,
               count
          };

          Void clear ( );

          AttackMode attack_mode;
          Uint8      key_count;
          Uint8      arrow_count;
          Uint8      bomb_count;
     };
}

#endif

