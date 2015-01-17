#ifndef BRYTE_CHARACTER_HPP
#define BRYTE_CHARACTER_HPP

#include "Vector.hpp"
#include "StopWatch.hpp"

namespace bryte
{
     class Map;

     enum Direction {
          left = 0,
          up,
          right,
          down,
          count
     };

     struct Character {
     public:

          enum State {
               dead,
               spawning,
               alive,
               blinking,
               attacking,
               dying,
          };

     public:

          Bool collides_with ( const Character& character );
          Bool attack_collides_with ( const Character& character );

          Void walk ( Direction dir );
          Void attack ( );
          Void damage ( Int32 amount, Direction push );

          Void update ( Real32 time_delta, const Map& map );

          Real32 calc_attack_x ( );
          Real32 calc_attack_y ( );

     public:

          static const Real32 c_damage_speed;
          static const Real32 c_damage_time;
          static const Real32 c_blink_time;
          static const Real32 c_attack_width;
          static const Real32 c_attack_height;
          static const Real32 c_attack_time;
          static const Real32 c_cooldown_time;
          static const Real32 c_accel;

     public:

          State     state;
          Direction facing;

          Stopwatch state_watch;

          Int32  health;
          Int32  max_health;

          Vector position;
          Vector velocity;
          Vector acceleration;

          Real32 width;
          Real32 height;

          Real32 collision_height;

          Direction damage_pushed;

          Stopwatch damage_watch;
          Stopwatch cooldown_watch;
     };
}

#endif

