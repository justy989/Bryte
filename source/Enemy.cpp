#include "Enemy.hpp"
#include "Utils.hpp"

using namespace bryte;

Void Enemy::init ( Type type )
{
     this->type = type;

     switch ( type ) {
     default:
          break;
     case Enemy::Type::rat:
          health     = 3;
          max_health = 3;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 1 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 14 ), pixels_to_meters ( 6 ) );

          rotate_collision = true;
          break;
     case Enemy::Type::bat:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 10 ), pixels_to_meters ( 6 ) );

          rotate_collision = false;
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
     }
}

Void Enemy::rat_think ( const Vector& player, Random& random, float time_delta )
{
     Bool&      moving = rat_state.moving;
     Stopwatch& timer  = rat_state.timer;

     if ( !moving ) {
          timer.tick ( time_delta );

          if ( timer.expired ( ) ) {
               // choose a new direction, and start moving
               facing = static_cast<Direction>( random.generate ( 0, Direction::count ) );
               timer.reset ( random.generate ( 0, 4 ) );
               moving = true;
          }
     } else {
          walk ( facing );

          timer.tick ( time_delta );

          if ( timer.expired ( ) || state == State::blinking ) {
               timer.reset ( random.generate ( 0, 3 ) );
               moving = false;
          }
     }
}

Void Enemy::bat_think ( const Vector& player, Random& random, float time_delta )
{
     Stopwatch& timer     = bat_state.timer;
     auto& move_direction = bat_state.move_direction;

     timer.tick ( time_delta );

     if ( timer.expired ( ) ) {
          move_direction = static_cast<BatState::Direction>( random.generate ( 0, BatState::Direction::count ) );
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
     case BatState::Direction::up:
          walk ( Direction::up );
          break;
     case BatState::Direction::down:
          walk ( Direction::down );
          break;
     case BatState::Direction::left:
          walk ( Direction::left );
          break;
     case BatState::Direction::right:
          walk ( Direction::right );
          break;     }

     facing = Direction::left;
}

