#ifndef BRYTE_ENTITY_MANAGER_HPP
#define BRYTE_ENTITY_MANAGER_HPP

#include "Entity.hpp"
#include "Utils.hpp"

namespace bryte
{
     template < typename E, Uint32 MAX >
     struct EntityManager {

          E* spawn ( const Vector& position );

          Void clear ( );

          E entities [ MAX ];

          inline E& operator[]( Uint32 i );

          inline Uint32 max ( ) const;
     };

     template < typename E, Uint32 MAX >
     E* EntityManager<E, MAX>::spawn ( const Vector& position )
     {
          for ( Uint32 i = 0; i < MAX; ++i ) {
               Auto& entity = entities [ i ];

               if ( entity.is_dead ( ) ) {
                    entity.life_state = Entity::LifeState::spawning;
                    entity.effected_by_element = Element::none;
                    entity.position = position;
                    return &entity;
               }
          }

          return nullptr;
     }

     template < typename E, Uint32 MAX >
     Void EntityManager<E, MAX>::clear ( )
     {
          for ( Uint32 i = 0; i < MAX; ++i ) {
               Auto& entity = entities [ i ];
               entity.life_state = Entity::LifeState::dead;
               entity.effected_by_element = Element::none;
               entity.position.set ( 0.0f, 0.0f );
               entity.clear ( );
          }
     }

     template < typename E, Uint32 MAX >
     inline E& EntityManager<E, MAX>::operator[]( Uint32 i )
     {
          ASSERT ( i < MAX );
          return entities [ i ];
     }

     template < typename E, Uint32 MAX >
     inline Uint32 EntityManager<E, MAX>::max ( ) const
     {
          return MAX;
     }

}
#endif

