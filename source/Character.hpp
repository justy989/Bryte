#ifndef BRYTE_CHARACTER_HPP
#define BRYTE_CHARACTER_HPP

#include "Direction.hpp"
#include "Vector.hpp"
#include "StopWatch.hpp"

namespace bryte
{
     class Map;
     class Interactives;

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

          Void update ( Real32 time_delta, const Map& map, Interactives& interactives );

          Real32 attack_x ( ) const;
          Real32 attack_y ( ) const;
          Real32 attack_width ( ) const;
          Real32 attack_height ( ) const;

          inline Real32 width ( ) const;
          inline Real32 height ( ) const;

          inline Real32 collision_x ( ) const;
          inline Real32 collision_y ( ) const;
          inline Real32 collision_x ( Real32 start_position ) const;
          inline Real32 collision_y ( Real32 start_position ) const;
          inline Real32 collision_width ( ) const;
          inline Real32 collision_height ( ) const;

          inline Real32 collision_center_x ( ) const;
          inline Real32 collision_center_y ( ) const;

          Void set_collision_center ( Real32 x, Real32 y );

     public:

          static const Real32 c_damage_accel;
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
          Vector dimension;
          Vector velocity;
          Vector acceleration;

          Vector collision_offset;
          Vector collision_dimension;

          Vector walk_acceleration;

          Direction damage_pushed;

          Stopwatch damage_watch;
          Stopwatch cooldown_watch;

          Bool collides_with_solids;
          Bool collides_with_exits;
     };

     inline Real32 Character::width ( ) const
     {
          return dimension.x ( );
     }

     inline Real32 Character::height ( ) const
     {
          return dimension.y ( );
     }

     inline Real32 Character::collision_x ( ) const
     {
          return position.x ( ) + collision_offset.x ( );
     }

     inline Real32 Character::collision_y ( ) const
     {
          return position.y ( ) + collision_offset.y ( );
     }

     inline Real32 Character::collision_x ( Real32 start_position ) const
     {
          return start_position + collision_offset.x ( );
     }

     inline Real32 Character::collision_y ( Real32 start_position ) const
     {
          return start_position + collision_offset.y ( );
     }

     inline Real32 Character::collision_width ( ) const
     {
          return collision_dimension.x ( );
     }

     inline Real32 Character::collision_height ( ) const
     {
          return collision_dimension.y ( );
     }

     inline Real32 Character::collision_center_x ( ) const
     {
          return collision_x ( ) + collision_width ( ) * 0.5f;
     }

     inline Real32 Character::collision_center_y ( ) const
     {
          return collision_y ( ) + collision_height ( ) * 0.5f;
     }
}

#endif

