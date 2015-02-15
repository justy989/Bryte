#include "Enemy.hpp"
#include "Utils.hpp"

#include <cmath>

using namespace bryte;

const Real32 Enemy::GooState::c_shoot_time = 2.0f;

Void Enemy::init ( Type type, Real32 x, Real32 y, Direction facing, Pickup::Type drop  )
{
     life_state   = LifeState::alive;
     state        = State::idle;

     this->type   = type;
     this->facing = facing;
     this->drop   = drop;

     position.set ( x, y );
     velocity.zero ( );
     acceleration.zero ( );

     damage_pushed = Direction::left;

     on_fire = false;

     state_watch.reset ( 0.0f );
     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );
     fire_watch.reset ( 0.0f );

     walk_tracker = 0.0f;
     walk_frame = 0;

     switch ( type ) {
     default:
          break;
     case Enemy::Type::rat:
          health     = 3;
          max_health = 3;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 1 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 12 ), pixels_to_meters ( 10 ) );

          collides_with_solids = true;
          collides_with_exits  = true;

          walk_acceleration = 9.0f;
          deceleration_scale = 5.0f;

          walk_frame_change = 0;
          walk_frame_count = 1;
          walk_frame_rate = 50.0f;
          constant_animation = false;
          break;
     case Enemy::Type::bat:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 10 ), pixels_to_meters ( 6 ) );

          collides_with_solids = true;
          collides_with_exits = true;

          walk_acceleration = 5.0f;
          deceleration_scale = 2.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 0.2f;
          constant_animation = true;
          break;
     case Enemy::Type::goo:
          health     = 4;
          max_health = 4;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 2 ) );
          collision_dimension.set ( pixels_to_meters ( 10 ), pixels_to_meters ( 10 ) );

          collides_with_solids = true;
          collides_with_exits = true;

          walk_acceleration = 3.5f;
          deceleration_scale = 2.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 0.4f;
          constant_animation = true;
          break;
     }
}

Void Enemy::think ( const Vector& player, Random& random, float time_delta )
{
     switch ( type )
     {
     default:
          break;
     case Type::rat:
          rat_think ( player, random, time_delta );
          break;
     case Type::bat:
          bat_think ( player, random, time_delta );
          break;
     case Type::goo:
          goo_think ( player, random, time_delta );
          break;
     }
}

Void Enemy::clear ( )
{
     state = State::idle;

     facing = Direction::left;
     state_watch.reset ( 0.0f );

     health = 0;
     max_health = 0;

     dimension.zero ( );
     acceleration.zero ( );

     collision_offset.zero ( );
     collision_dimension.zero ( );

     walk_acceleration = 0.0f;

     damage_pushed = Direction::count;

     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );

     position.zero ( );

     collides_with_solids = false;
     collides_with_exits = false;

     type = Type::count;
     drop = Pickup::Type::none;
}

Void Enemy::rat_think ( const Vector& player, Random& random, float time_delta )
{
     Bool& moving             = rat_state.moving;
     Bool& reacting_to_attack = rat_state.reacting_to_attack;
     Stopwatch& timer         = rat_state.timer;

     // if attacked move in a
     if ( state == Character::State::blinking && !reacting_to_attack ) {
          facing = static_cast<Direction>( random.generate ( 0, Direction::count ) );
          timer.reset ( random.generate ( 1, 3 ) );
          moving             = true;
          reacting_to_attack = true;
     }

     if ( !moving ) {
          timer.tick ( time_delta );

          if ( timer.expired ( ) ) {
               // choose a new direction, and start moving
               facing = static_cast<Direction>( random.generate ( 0, Direction::count ) );
               timer.reset ( random.generate ( 0, 4 ) );
               moving = true;
          }
     } else {
          if ( collided_last_frame ) {
               moving = false;
               timer.reset ( 0.0f );
          } else {
               walk ( facing );

               timer.tick ( time_delta );

               if ( timer.expired ( ) ) {
                    timer.reset ( random.generate ( 0, 3 ) );
                    moving             = false;
                    reacting_to_attack = false;
               }
          }
     }
}

Void Enemy::bat_think ( const Vector& player, Random& random, float time_delta )
{
     Stopwatch& timer     = bat_state.timer;
     Auto& move_direction = bat_state.move_direction;

     timer.tick ( time_delta );

     if ( timer.expired ( ) || collided_last_frame ) {
          move_direction = static_cast<BatState::Direction>( random.generate ( 0, BatState::Direction::count + 1 ) );
          timer.reset ( random.generate ( 1, 2 ) );
     }

     switch ( move_direction ) {
     default:
          break;
     case BatState::Direction::up_left:
          walk ( Direction::up );
          walk ( Direction::left );
          break;
     case BatState::Direction::up_right:
          walk ( Direction::up );
          walk ( Direction::right );
          break;
     case BatState::Direction::down_left:
          walk ( Direction::down );
          walk ( Direction::left );
          break;
     case BatState::Direction::down_right:
          walk ( Direction::down );
          walk ( Direction::right );
          break;
     }

     // purely since the bat bitmap only has 1 direction
     facing = Direction::left;
}

Void Enemy::goo_think ( const Vector& player, Random& random, float time_delta )
{
     GooState::State& state = goo_state.state;
     Stopwatch& state_timer = goo_state.state_timer;

     state_timer.tick ( time_delta );

     switch ( state ) {
     default:
          break;
     case GooState::State::walking:
          if ( collided_last_frame ) {
               state = GooState::State::picking_direction;
          } else {
               walk ( facing );

               if ( state_timer.expired ( ) ) {
                    state = GooState::State::preparing_to_shoot;
                    state_timer.reset ( GooState::c_shoot_time );
               }
          }
          break;
     case GooState::State::preparing_to_shoot:
          if ( state_timer.expired ( ) ) {
               state = GooState::State::shooting;
          }
          break;
     case GooState::State::shooting:
          state = GooState::State::picking_direction;
          break;
     case GooState::State::picking_direction:
          facing = static_cast<Direction>( random.generate ( 0, Direction::count ) );
          state_timer.reset ( random.generate ( 2, 4 ) );
          state= GooState::State::walking;
          break;
     }
}

