#ifndef BRYTE_ENEMY_HPP
#define BRYTE_ENEMY_HPP

#include "Character.hpp"
#include "Random.hpp"

namespace bryte
{
     struct Enemy : public Character {
     public:

          enum Type {
               rat,
               bat,
               count
          };

          struct RatState {
               bool      moving; // either moving or waiting
               Stopwatch timer;
          };

          struct BatState {
               enum Direction {
                    up_left,
                    up_right,
                    down_left,
                    down_right,
                    up,
                    down,
                    left,
                    right,
                    count
               };

               Direction move_direction;
               Stopwatch timer;
          };

          Void init ( Type type );

     public:

          Void think ( const Vector& player, Random& random, float time_delta );

     private:

          Void rat_think ( const Vector& player, Random& random, float time_delta );
          Void bat_think ( const Vector& player, Random& random, float time_delta );

     public:

          Type type;

          union {
               RatState rat_state;
               BatState bat_state;
          };
     };
}

#endif

