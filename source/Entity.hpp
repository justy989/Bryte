#ifndef BRYTE_ENTITY_HPP
#define BRYTE_ENTITY_HPP

#include "Vector.hpp"
#include "Element.hpp"

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
          inline Bool is_dying ( ) const;
          inline Bool is_spawning ( ) const;
          inline Bool is_alive ( ) const;

          Vector    position;
          LifeState life_state;
          Bool      on_ice;
          Element   effected_by_element;
     };

     inline Bool Entity::is_dead ( ) const
     {
          return life_state == LifeState::dead;
     }

     inline Bool Entity::is_dying ( ) const
     {
          return life_state == LifeState::dying;
     }

     inline Bool Entity::is_spawning ( ) const
     {
          return life_state == LifeState::spawning;
     }

     inline Bool Entity::is_alive ( ) const
     {
          return life_state == LifeState::alive;
     }

     struct TrackEntity {
          Entity* entity;
          Vector offset;
     };
}

#endif

