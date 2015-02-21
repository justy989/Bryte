#ifndef BRYTE_CHARACTER_HPP
#define BRYTE_CHARACTER_HPP

#include "Direction.hpp"
#include "Entity.hpp"
#include "StopWatch.hpp"

namespace bryte
{
     class Map;
     class Interactives;

     struct Character : public Entity {
     public:

          enum State {
               idle,
               blinking,
               attacking,
               pushing,
               blocking
          };

     public:

          Bool collides_with ( const Character& character );
          Bool attack_collides_with ( const Character& character );
          Bool in_tile ( Int32 x, Int32 y ) const;

          Void walk ( Direction dir );
          Bool attack ( );
          Void block ( );
          Void damage ( Int32 amount, Direction push );
          Void light_on_fire ( );

          Void update ( Real32 time_delta, const Map& map, Interactives& interactives );

          Real32 attack_x ( ) const;
          Real32 attack_y ( ) const;
          Real32 attack_width ( ) const;
          Real32 attack_height ( ) const;
          Int32  attack_width_in_pixels ( ) const;
          Int32  attack_height_in_pixels ( ) const;

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

          inline Vector collision_center ( ) const;

          inline Bool is_idle ( ) const;
          inline Bool is_blinking ( ) const;
          inline Bool is_attacking ( ) const;
          inline Bool is_pushing ( ) const;
          inline Bool is_blocking ( ) const;

          Void set_collision_center ( Real32 x, Real32 y );

     public:

          static const Real32 c_damage_accel;
          static const Real32 c_damage_time;
          static const Real32 c_blink_time;
          static const Int32  c_attack_width_in_pixels;
          static const Int32  c_attack_height_in_pixels;
          static const Real32 c_attack_width_in_meters;
          static const Real32 c_attack_height_in_meters;
          static const Real32 c_attack_time;
          static const Real32 c_cooldown_time;
          static const Real32 c_dying_time;
          static const Real32 c_accel;

          static const Int32  c_fire_tick_max;
          static const Real32 c_fire_tick_rate;

          static const Real32 c_ice_decel;

          static const Real32 c_moving_walkway_accel;

     public:

          State     state;
          Direction facing;

          Stopwatch state_watch;

          Int32  health;
          Int32  max_health;

          Vector dimension;
          Vector velocity;
          Vector acceleration;

          Real32 deceleration_scale;

          Vector collision_offset;
          Vector collision_dimension;

          Real32 walk_acceleration;

          Bool collided_last_frame;

          Int8 walk_frame;
          Int8 walk_frame_change;
          Int8 walk_frame_count;

          Real32 walk_tracker;
          Real32 walk_frame_rate;

          Bool constant_animation;

          Direction damage_pushed;

          Stopwatch damage_watch;
          Stopwatch cooldown_watch;
          Stopwatch fire_watch;

          Uint8 fire_tick_count;

          Bool collides_with_solids;
          Bool collides_with_exits;

          Bool on_ice;
          Direction on_moving_walkway;
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

     inline Vector Character::collision_center ( ) const
     {
          return Vector { collision_center_x ( ), collision_center_y ( ) };
     }

     inline Bool Character::is_idle ( ) const { return state == Character::State::idle; }
     inline Bool Character::is_blinking ( ) const { return state == Character::State::blinking; }
     inline Bool Character::is_attacking ( ) const { return state == Character::State::attacking; }
     inline Bool Character::is_pushing ( ) const { return state == Character::State::pushing; }
     inline Bool Character::is_blocking ( ) const { return state == Character::State::blocking; }
}

#endif

