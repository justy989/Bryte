#ifndef BRYTE_PLAYER_HPP
#define BRYTE_PLAYER_HPP

#include "Character.hpp"

namespace bryte
{
     struct Player : public Character {
     public:

          enum ItemMode {
               no_item,
               arrow,
               bomb,
               count
          };

          enum Upgrade {
               none,
               wooden_sword,
               magic_sword,
               iron_shield,
               mirror_shield,
               bow,
               bomb_bag,
               heart,
          };

          enum Sword {
               no_sword,
               wooden,
               magic,
          };

          enum Shield {
               no_shield,
               iron,
               mirror,
          };

          Void clear ( );

          Bool save ( );
          Bool load ( );

          Void give_arrow ( );
          Void give_bomb ( );

          Void give_upgrade ( Upgrade upgrade );

          Bool use_bow ( );
          Bool use_bomb ( );

          Bool can_attack ( );

     public:

          static const Real32 c_item_cooldown;

          static const Int32 c_arrow_increment = 3;
          static const Int32 c_bomb_increment = 1;
          static const Int32 c_max_health_increment = 2;
          static const Int32 c_max_arrow_increment = 5;
          static const Int32 c_max_bomb_increment = 2;

          static const Char8* c_upgrade_names [ Upgrade::heart + 1 ];

     public:

          ItemMode item_mode;
          Stopwatch item_cooldown;

          Uint8 key_count;

          Uint8 arrow_count;
          Uint8 max_arrows;

          Uint8 bomb_count;
          Uint8 max_bombs;

          Sword sword;
          Shield shield;
          Bool has_bow;

          Uint8 save_slot;
     };
}

#endif

