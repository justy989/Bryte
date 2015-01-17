#include "Enemy.hpp"

using namespace bryte;

Void Enemy::think ( const Vector& player, Random& random, float time_delta )
{
     switch ( type )
     {
     default:
          break;
     case Type::rat:
          rat_think ( player, random, time_delta );
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
               timer.reset ( random.generate ( 0, 3 ) );
               moving = true;
          }
     } else {
          walk ( facing );

          timer.tick ( time_delta );

          if ( timer.expired ( ) ) {
               timer.reset ( random.generate ( 0, 2 ) );
               moving = false;
          }
     }
}

