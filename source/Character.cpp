#include "Character.hpp"
#include "Map.hpp"
#include "Interactives.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Character::c_damage_accel            = 40.0f;
const Real32 Character::c_damage_time             = 0.15f;
const Real32 Character::c_blink_time              = 1.5f;
const Int32  Character::c_attack_width_in_pixels  = 9;
const Int32  Character::c_attack_height_in_pixels = 4;
const Real32 Character::c_attack_width_in_meters  = pixels_to_meters ( Character::c_attack_width_in_pixels );
const Real32 Character::c_attack_height_in_meters = pixels_to_meters ( Character::c_attack_height_in_pixels );
const Real32 Character::c_attack_time             = 0.35f;
const Real32 Character::c_cooldown_time           = 0.25f;

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
          return position.x ( ) - Character::c_attack_height_in_meters;
     case Direction::right:
          return position.x ( ) + dimension.x ( ) * 0.8f;
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
          return c_attack_width_in_meters;
     case Direction::up:
     case Direction::down:
          return c_attack_height_in_meters;
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
          return c_attack_height_in_meters;
     case Direction::up:
     case Direction::down:
          return c_attack_width_in_meters;
     }

     return 0.0f;
}

Int32 Character::attack_width_in_pixels ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
     case Direction::right:
          return c_attack_width_in_pixels;
     case Direction::up:
     case Direction::down:
          return c_attack_height_in_pixels;
     }

     return 0.0f;
}

Int32 Character::attack_height_in_pixels ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
     case Direction::right:
          return c_attack_height_in_pixels;
     case Direction::up:
     case Direction::down:
          return c_attack_width_in_pixels;
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

static Bool check_wall ( Real32 wall, Real32 wall_min, Real32 wall_max,
                         Real32 starting_position, Real32 change_in_position,
                         Real32 other_starting_position, Real32 other_change_in_position,
                         Real32* closest_time_intersection )
{
     if ( change_in_position == 0.0f ) {
          return false;
     }

     Real32 time_intersection = ( wall - starting_position ) / change_in_position;

     if ( time_intersection > 0.0f && time_intersection < 1.0f ) {

          if ( *closest_time_intersection > time_intersection ) {

               Real32 other_intersection = other_starting_position + time_intersection * other_change_in_position;

               if ( other_intersection >= wall_min && other_intersection <= wall_max ) {
                    *closest_time_intersection = time_intersection;
                    return true;
               }
          }
     }

     return false;
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
     } break;
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
     acceleration += velocity * -4.0f;

     Vector change_in_position = ( velocity * time_delta ) +
                                 ( acceleration * ( 0.5f * square ( time_delta ) ) );

     velocity = acceleration * time_delta + velocity;

     Real32 half_width  = collision_width ( ) * 0.5f;
     Real32 half_height = collision_height ( ) * 0.5f;

     Vector center { collision_x ( ) + half_width,
                     collision_y ( ) + half_height };

     Int32 center_tile_x = meters_to_pixels ( center.x ( ) ) / Map::c_tile_dimension_in_pixels;
     Int32 center_tile_y = meters_to_pixels ( center.y ( ) ) / Map::c_tile_dimension_in_pixels;

     // TODO: move tiles based on width
     Int32 min_check_tile_x = center_tile_x - 1;
     Int32 min_check_tile_y = center_tile_y - 1;
     Int32 max_check_tile_x = center_tile_x + 1;
     Int32 max_check_tile_y = center_tile_y + 1;

     CLAMP ( min_check_tile_x, 0, map.width  ( ) - 1 );
     CLAMP ( min_check_tile_y, 0, map.height ( ) - 1 );
     CLAMP ( max_check_tile_x, 0, map.width  ( ) - 1 );
     CLAMP ( max_check_tile_y, 0, map.height ( ) - 1 );

     Real32 time_remaining = 1.0f;

     for ( int i = 0; i < 4 && time_remaining > 0.0f; ++i ) {
          Vector wall_normal;
          Real32 closest_time_intersection = time_remaining;
          Direction push_direction;
          Int32 push_interactive_x = -1;
          Int32 push_interactive_y = -1;

          // loop over tile area
          for ( Int32 y = min_check_tile_y; y <= max_check_tile_y; ++y ) {
               for ( Int32 x = min_check_tile_x; x <= max_check_tile_x; ++x ) {
                    auto& interactive = interactives.get_from_tile ( x, y );

                    if ( !map.get_coordinate_solid ( x, y ) &&
                         !interactive.is_solid ( ) ) {
                         if ( interactive.type == Interactive::Type::exit ) {
                              if ( !collides_with_exits ) {
                                   continue;
                              }
                         } else {
                              continue;
                         }
                    }

                    Real32 left   = pixels_to_meters ( x * Map::c_tile_dimension_in_pixels );
                    Real32 right  = left + Map::c_tile_dimension_in_meters;
                    Real32 bottom = pixels_to_meters ( y * Map::c_tile_dimension_in_pixels );
                    Real32 top    = bottom + Map::c_tile_dimension_in_meters;

                    // minkowski sum extruding
                    left   -= half_width;
                    right  += half_width;
                    bottom -= half_height;
                    top    += half_height;

                    if ( check_wall ( left, bottom, top, center.x ( ), change_in_position.x ( ),
                                      center.y ( ), change_in_position.y ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { -1.0f, 0.0f };
                         if ( interactive.is_solid ( ) ) {
                              push_direction = Direction::right;
                              push_interactive_x = x;
                              push_interactive_y = y;
                         }
                    }

                    if ( check_wall ( right, bottom, top, center.x ( ), change_in_position.x ( ),
                                      center.y ( ), change_in_position.y ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { 1.0f, 0.0f };
                         if ( interactive.is_solid ( ) ) {
                              push_direction = Direction::left;
                              push_interactive_x = x;
                              push_interactive_y = y;
                         }
                    }

                    if ( check_wall ( bottom, left, right, center.y ( ), change_in_position.y ( ),
                                      center.x ( ), change_in_position.x ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { 0.0f, -1.0f };
                         if ( interactive.is_solid ( ) ) {
                              push_direction = Direction::up;
                              push_interactive_x = x;
                              push_interactive_y = y;
                         }
                    }

                    if ( check_wall ( top, left, right, center.y ( ), change_in_position.y ( ),
                                      center.x ( ), change_in_position.x ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { 0.0f, 1.0f };

                         if ( interactive.is_solid ( ) ) {
                              push_direction = Direction::down;
                              push_interactive_x = x;
                              push_interactive_y = y;
                         }
                    }
              }
          }

          // push any interactives we are colliding with
          if ( push_interactive_x >= 0 ) {
               interactives.push ( push_interactive_x, push_interactive_y, push_direction, map );
          }

          position += ( change_in_position * ( closest_time_intersection - 0.01f ) );
          velocity -= ( wall_normal * velocity.inner_product ( wall_normal ) );
          change_in_position -= ( wall_normal * change_in_position.inner_product ( wall_normal ) );

          time_remaining -= closest_time_intersection;
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
          acceleration += Vector ( -walk_acceleration.x ( ), 0.0f );
          break;
     case Direction::up:
          acceleration += Vector ( 0.0f, walk_acceleration.y ( ) );
          break;
     case Direction::right:
          acceleration += Vector ( walk_acceleration.x ( ), 0.0f );
          break;
     case Direction::down:
          acceleration += Vector ( 0.0f, -walk_acceleration.y ( ) );
          break;
     }

     facing = dir;
}

Void Character::set_collision_center ( Real32 x, Real32 y )
{
     Real32 x_offset = collision_center_x ( ) - position.x ( );
     Real32 y_offset = collision_center_y ( ) - position.y ( );

     position = Vector{ x - x_offset, y - y_offset };
}

