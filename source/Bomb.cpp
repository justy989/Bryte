#include "Bomb.hpp"

using namespace bryte;

// NOTE: Windows compiler does static initialization different from gcc
//       so I cannot have the explode radius rely on map tile dimension
const Real32 Bomb::c_explode_time = 3.0f;
const Real32 Bomb::c_explode_radius = 3.2f;

Void Bomb::update ( float dt )
{
     switch ( life_state ) {
     default:
          break;
     case LifeState::spawning:
          life_state = LifeState::alive;
          break;
     case Entity::LifeState::alive:
          explode_watch.tick ( dt );

          // allow 1 frame where the bomb has expired but isn't dead
          if ( explode_watch.expired ( ) ) {
               life_state = Entity::LifeState::dying;
          }
          break;
     case Entity::LifeState::dying:
          life_state = Entity::LifeState::dead;
          break;
     }
}

Void Bomb::clear ( )
{
     explode_watch.reset ( 0.0f );
}


