#ifndef BRYTE_ENTITY_HPP
#define BRYTE_ENTITY_HPP

#include "Vector.hpp"

namespace bryte
{
     struct Entity {
          enum LifeState {
               dead,
               dying,
               spawning,
               alive
          };

          inline Bool is_dead ( ) const;

          Vector    position;
          LifeState life_state;
     };

     inline Bool Entity::is_dead ( ) const
     {
          return life_state == LifeState::dead;
     }
}

#endif

