#ifndef BRYTE_ENEMY_HPP
#define BRYTE_ENEMY_HPP

#include "Character.hpp"
#include "Pickup.hpp"
#include "Random.hpp"

namespace bryte
{
     struct Enemy : public Character {
     public:

          enum Type {
               rat,
               bat,
               goo,
               skeleton,
               count
          };

          struct RatState {
               bool      moving; // either moving or waiting
               bool      reacting_to_attack;
               Stopwatch timer;
          };

          struct BatState {
               enum Direction {
                    up_left,
                    up_right,
                    down_left,
                    down_right,
                    count
               };

               Direction move_direction;
               Stopwatch timer;
          };

          struct GooState {
               enum State {
                    walking,
                    picking_direction,
                    preparing_to_shoot,
                    shooting
               };

               static const Real32 c_shoot_time;

               State state;
               Stopwatch state_timer;
          };

          struct SkeletonState {
               static const Real32 c_attack_range;

               Stopwatch wander_timer;
               Direction wander_direction; // TODO: get rid of once we allow single line animation
          };

     public:

          Void init ( Type type, Real32 x, Real32 y, Direction facing, Pickup::Type drop );

          Void think ( const Vector& player, Random& random, float time_delta );

          Void clear ( );

     private:

          Void rat_think ( const Vector& player, Random& random, float time_delta );
          Void bat_think ( const Vector& player, Random& random, float time_delta );
          Void goo_think ( const Vector& player, Random& random, float time_delta );
          Void skeleton_think ( const Vector& player, Random& random, float time_delta );

     public:

          Type type;

          union {
               RatState rat_state;
               BatState bat_state;
               GooState goo_state;
               SkeletonState skeleton_state;
          };

          Pickup::Type drop;
     };
}

#endif

