#include "DamageNumber.hpp"

using namespace bryte;

const Real32 DamageNumber::c_rise_height = 1.0f;
const Real32 DamageNumber::c_rise_speed = 1.0f;

Void DamageNumber::update ( float time_delta )
{
     position += Vector ( 0.0f, c_rise_speed * time_delta );

     if ( ( position.y ( ) - starting_y ) > c_rise_height ) {
          life_state = Entity::LifeState::dead;
     }
}

Void DamageNumber::clear ( )
{
     value = 0;
     starting_y = 0.0f;
     position.zero ( );
}

