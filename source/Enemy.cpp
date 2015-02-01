#include "Enemy.hpp"
#include "Utils.hpp"

using namespace bryte;

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

     state_watch.reset ( 0.0f );
     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );

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

          walk_acceleration.set ( 9.0f, 9.0f );
          break;
     case Enemy::Type::bat:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 10 ), pixels_to_meters ( 6 ) );

          collides_with_solids = true;
          collides_with_exits = true;

          walk_acceleration.set ( 5.0f, 5.0f );
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

     walk_acceleration.zero ( );

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
          walk ( facing );

          timer.tick ( time_delta );

          if ( timer.expired ( ) ) {
               timer.reset ( random.generate ( 0, 3 ) );
               moving             = false;
               reacting_to_attack = false;
          }
     }
}

Void Enemy::bat_think ( const Vector& player, Random& random, float time_delta )
{
     Stopwatch& timer     = bat_state.timer;
     Auto& move_direction = bat_state.move_direction;

     timer.tick ( time_delta );

     if ( timer.expired ( ) ) {
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

     facing = Direction::left;
}

