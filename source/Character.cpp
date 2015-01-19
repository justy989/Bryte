#include "Character.hpp"
#include "Map.hpp"
#include "Interactives.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Character::c_damage_accel  = 80.0f;
const Real32 Character::c_damage_time   = 0.15f;
const Real32 Character::c_blink_time    = 1.5f;
const Real32 Character::c_attack_width  = 0.7f;
const Real32 Character::c_attack_height = 0.3f;
const Real32 Character::c_attack_time   = 0.35f;
const Real32 Character::c_cooldown_time = 0.25f;
const Real32 Character::c_accel         = 7.0f;

Bool Character::collides_with ( const Character& character )
{
     return rect_collides_with_rect ( collision_x ( ), collision_y ( ), collision_width ( ), collision_height ( ),
                                      character.collision_x ( ), character.collision_y ( ),
                                      character.collision_width ( ), character.collision_height ( ) );
}

Void Character::attack ( )
{
     if ( state != State::alive || !cooldown_watch.expired ( ) ) {
          return;
     }

     state_watch.reset ( Character::c_attack_time );

     state = State::attacking;
}

Real32 Character::attack_x ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
          return position.x ( ) - Character::c_attack_height;
     case Direction::right:
          return position.x ( ) + dimension.x ( );
     case Direction::up:
          return position.x ( ) + dimension.x ( ) * 0.33f;
     case Direction::down:
          return position.x ( ) + dimension.x ( ) * 0.33f;
     }

     return 0.0f;
}

Real32 Character::attack_y ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
          return position.y ( ) + dimension.y ( ) * 0.2f;
     case Direction::right:
          return position.y ( ) + dimension.y ( ) * 0.2f;
     case Direction::up:
          return position.y ( ) + dimension.y ( );
     case Direction::down:
          return position.y ( ) - dimension.y ( ) * 0.5f;
     }

     return 0.0f;
}

Real32 Character::attack_width ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
     case Direction::right:
          return c_attack_width;
     case Direction::up:
     case Direction::down:
          return c_attack_height;
     }

     return 0.0f;
}

Real32 Character::attack_height ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
     case Direction::right:
          return c_attack_height;
     case Direction::up:
     case Direction::down:
          return c_attack_width;
     }

     return 0.0f;
}

Bool Character::attack_collides_with ( const Character& character )
{
     return rect_collides_with_rect ( attack_x ( ), attack_y ( ),
                                      attack_width ( ), attack_height ( ),
                                      character.collision_x ( ), character.collision_y ( ),
                                      character.collision_width ( ), character.collision_height ( ) );
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

Void Character::update ( Real32 time_delta, const Map& map, Interactives& interactives )
{
     // tick stopwatches
     state_watch.tick ( time_delta );
     cooldown_watch.tick ( time_delta );

     // logic based on current state
     switch ( state ) {
     case State::blinking:
     {
          Vector damage_velocity;

          damage_watch.tick ( time_delta );

          if ( !damage_watch.expired ( ) ) {
               switch ( damage_pushed ) {
               default:
                    ASSERT ( 0 );
               case Direction::left:
                    acceleration += Vector( -1.0f, 0.0f ) * c_damage_accel;
                    break;
               case Direction::right:
                    acceleration += Vector( 1.0f, 0.0f ) * c_damage_accel;
                    break;
               case Direction::up:
                    acceleration += Vector( 0.0f, 1.0f ) * c_damage_accel;
                    break;
               case Direction::down:
                    acceleration += Vector( 0.0f, -1.0f ) * c_damage_accel;
                    break;
               }
          }

          if ( state_watch.expired ( ) ) {
               if ( state != State::dead ) {
                    state = State::alive;
               }
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

     // TEMPORARY, slow character down
     acceleration += velocity * -2.0f;

     Vector target_position = position + ( velocity * time_delta ) +
                              ( acceleration * ( 0.5f * square ( time_delta ) ) );
     velocity = acceleration * time_delta + velocity;

     bool collided = false;

     // which tile did our target_position end up in
     Int32 starting_tile_left   = collision_x ( ) / Map::c_tile_dimension_in_meters;
     Int32 starting_tile_right  = ( collision_x ( ) + collision_width ( ) ) / Map::c_tile_dimension_in_meters;
     Int32 starting_tile_bottom =  collision_y ( ) / Map::c_tile_dimension_in_meters;
     Int32 starting_tile_top    = ( collision_y ( ) + collision_height ( ) ) / Map::c_tile_dimension_in_meters;

     Int32 target_tile_left     = collision_x ( target_position.x ( ) ) / Map::c_tile_dimension_in_meters;
     Int32 target_tile_right    = ( collision_x ( target_position.x ( ) ) + collision_width ( ) ) /
                                  Map::c_tile_dimension_in_meters;
     Int32 target_tile_bottom   = collision_y ( target_position.y ( ) ) / Map::c_tile_dimension_in_meters;
     Int32 target_tile_top      = ( collision_y ( target_position.y ( ) ) + collision_height ( ) ) /
                                  Map::c_tile_dimension_in_meters;

     // check collision with tile map
     if ( map.get_coordinate_solid ( target_tile_left, target_tile_bottom ) ||
          map.get_coordinate_solid ( target_tile_right, target_tile_bottom ) ||
          map.get_coordinate_solid ( target_tile_left, target_tile_top ) ||
          map.get_coordinate_solid ( target_tile_right, target_tile_top ) ) {
          collided = true;
     }

     const Interactive& bottom_left_interactive  = interactives.interactive ( target_tile_left,
                                                                              target_tile_bottom );
     const Interactive& top_left_interactive     = interactives.interactive ( target_tile_left,
                                                                              target_tile_top );
     const Interactive& bottom_right_interactive = interactives.interactive ( target_tile_right,
                                                                              target_tile_bottom );
     const Interactive& top_right_interactive    = interactives.interactive ( target_tile_right,
                                                                              target_tile_top );

     if ( bottom_left_interactive.is_solid ( ) || top_left_interactive.is_solid ( ) ||
          bottom_right_interactive.is_solid ( ) || top_right_interactive.is_solid ( ) ) {
          collided = true;
     }

     Vector wall;

     if ( starting_tile_left - target_tile_left > 0 ) {
          wall.set ( 1.0f, 0.0f );
          interactives.push ( target_tile_left, target_tile_bottom, Direction::left, map );
     } else if ( starting_tile_right - target_tile_right < 0 ) {
          wall.set ( -1.0f, 0.0f );
          interactives.push ( target_tile_right, target_tile_bottom, Direction::right, map );
     }

     if ( starting_tile_bottom - target_tile_bottom > 0 ) {
          wall.set ( 0.0f, 1.0f );
          interactives.push ( target_tile_left, target_tile_bottom, Direction::down, map );
     } else if ( starting_tile_top - target_tile_top < 0 ) {
          wall.set ( 0.0f, -1.0f );
          interactives.push ( target_tile_left, target_tile_top, Direction::up, map );
     }

     if ( !collided ) {
          position = target_position;
     } else {
          velocity = velocity - ( wall * velocity.inner_product ( wall ) ); // slidy
          //velocity = velocity - ( wall * velocity.inner_product ( wall ) * 2.0f ); // bouncy
     }

     acceleration.zero ( );
}

Void Character::walk ( Direction dir )
{
     switch ( dir ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          acceleration += Vector( -1.0f, 0.0f ) * c_accel;
          break;
     case Direction::up:
          acceleration += Vector( 0.0f, 1.0f ) * c_accel;
          break;
     case Direction::right:
          acceleration += Vector( 1.0f, 0.0f ) * c_accel;
          break;
     case Direction::down:
          acceleration += Vector( 0.0f, -1.0f ) * c_accel;
          break;
     }

     facing = dir;
}

Real32 Character::collision_x ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
     case Direction::right:
          return position.x ( ) + collision_offset.x ( );
     case Direction::up:
     case Direction::down:
          if ( rotate_collision ) {
               return position.x ( ) + collision_offset.y ( );
          }

          return position.x ( ) + collision_offset.x ( );
     }

     return 0.0f;
}

Real32 Character::collision_y ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
     case Direction::right:
          return position.y ( ) + collision_offset.y ( );
     case Direction::up:
     case Direction::down:
          if ( rotate_collision ) {
               return position.y ( ) + collision_offset.x ( );
          }

          return position.y ( ) + collision_offset.y ( );
     }

     return 0.0f;
}

Real32 Character::collision_x ( Real32 start_position ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
     case Direction::right:
          return start_position + collision_offset.x ( );
     case Direction::up:
     case Direction::down:
          if ( rotate_collision ) {
               return start_position + collision_offset.y ( );
          }

          return start_position + collision_offset.x ( );
     }

     return 0.0f;
}

Real32 Character::collision_y ( Real32 start_position ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
     case Direction::right:
          return start_position + collision_offset.y ( );
     case Direction::up:
     case Direction::down:
          if ( rotate_collision ) {
               return start_position + collision_offset.x ( );
          }

          return start_position + collision_offset.y ( );
     }

     return 0.0f;
}

Real32 Character::collision_width ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
     case Direction::right:
          return collision_dimension.x ( );
     case Direction::up:
     case Direction::down:
          if ( rotate_collision ) {
               return collision_dimension.y ( );
          }

          return collision_dimension.x ( );
     }

     return 0.0f;
}

Real32 Character::collision_height ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
     case Direction::right:
          return collision_dimension.y ( );
     case Direction::up:
     case Direction::down:
          if ( rotate_collision ) {
               return collision_dimension.x ( );
          }

          return collision_dimension.y ( );
     }

     return 0.0f;
}

