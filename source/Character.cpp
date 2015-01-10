#include "Character.hpp"
#include "Map.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Character::c_damage_speed  = 10.0f;
const Real32 Character::c_damage_time   = 0.15f;
const Real32 Character::c_blink_time    = 1.5f;
const Real32 Character::c_attack_width  = 0.6f;
const Real32 Character::c_attack_height = 1.2f;
const Real32 Character::c_attack_time   = 0.35f;
const Real32 Character::c_cooldown_time = 0.25f;

Bool Character::collides_with ( const Character& character )
{
     return rect_collides_with_rect ( position_x, position_y, width, collision_height,
                                      character.position_x, character.position_y,
                                      character.width, character.collision_height );
}

Void Character::attack ( )
{
     if ( state != State::alive || !cooldown_watch.expired ( ) ) {
          return;
     }

     state_watch.reset ( Character::c_attack_time );

     state = State::attacking;
}

Real32 Character::calc_attack_x ( )
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
          return position_x - Character::c_attack_height;
     case Direction::right:
          return position_x + width;
     case Direction::up:
          return position_x + width * 0.33f;
     case Direction::down:
          return position_x + width * 0.33f;
     }

     return 0.0f;
}

Real32 Character::calc_attack_y ( )
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
          return position_y + height * 0.5f;
     case Direction::right:
          return position_y + height * 0.5f;
     case Direction::up:
          return position_y + height;
     case Direction::down:
          return position_y - height * 0.5f;
     }

     return 0.0f;
}

Bool Character::attack_collides_with ( const Character& character )
{
     // swap width/height based on direction we are facing
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::up:
     case Direction::down:
          return rect_collides_with_rect ( calc_attack_x ( ), calc_attack_y ( ),
                                           Character::c_attack_width, Character::c_attack_height,
                                           character.position_x, character.position_y,
                                           character.width, character.height );
     case Direction::left:
     case Direction::right:
          return rect_collides_with_rect ( calc_attack_x ( ), calc_attack_y ( ),
                                           Character::c_attack_height, Character::c_attack_width,
                                           character.position_x, character.position_y,
                                           character.width, character.height );
     }

     return false;
}

Void Character::damage ( Int32 amount, Direction push )
{
     health -= amount;

     if ( health > 0 ) {
          damage_pushed = push;

          damage_watch.reset ( Character::c_damage_time );
          state_watch.reset ( Character::c_blink_time );

          state = State::blinking;
     } else {
          // TODO: change to dying and handle transition to death
          state = State::dead;
     }
}

Void Character::update ( Real32 time_delta, const Map& map )
{
     Real32 target_position_x = position_x + velocity_x * time_delta;
     Real32 target_position_y = position_y + velocity_y * time_delta;

     // tick stopwatches
     state_watch.tick ( time_delta );
     cooldown_watch.tick ( time_delta );

     // logic based on current state
     switch ( state ) {
     case State::blinking:

          damage_watch.tick ( time_delta );

          if ( !damage_watch.expired ( ) ) {
               switch ( damage_pushed ) {
               default:
                    ASSERT ( 0 );
               case Direction::left:
                    target_position_x -= Character::c_damage_speed * time_delta;
                    break;
               case Direction::right:
                    target_position_x += Character::c_damage_speed * time_delta;
                    break;
               case Direction::up:
                    target_position_y += Character::c_damage_speed * time_delta;
                    break;
               case Direction::down:
                    target_position_y -= Character::c_damage_speed * time_delta;
                    break;
               }
          }

          if ( state_watch.expired ( ) ) {
               if ( state != State::dead ) {
                    state = State::alive;
               }
          }

          break;
     case State::attacking:
          if ( state_watch.expired ( ) ) {
               cooldown_watch.reset ( Character::c_cooldown_time );
               state = State::alive;
          }

          break;
     default:
          break;
     }

     bool collided = false;

     // collision with tile map
     if ( map.is_position_solid ( target_position_x, target_position_y ) ||
          map.is_position_solid ( target_position_x + width, target_position_y ) ||
          map.is_position_solid ( target_position_x, target_position_y + collision_height ) ||
          map.is_position_solid ( target_position_x + width, target_position_y + collision_height ) ) {
          collided = true;
     }

     if ( !collided ) {
          position_x = target_position_x;
          position_y = target_position_y;
     }

     velocity_x = 0.0f;
     velocity_y = 0.0f;
}
