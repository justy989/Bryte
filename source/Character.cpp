#include "Character.hpp"
#include "Map.hpp"
#include "Interactives.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Character::c_damage_accel            = 65.0f;
const Real32 Character::c_damage_time             = 0.15f;
const Real32 Character::c_blink_time              = 1.5f;
const Int32  Character::c_attack_width_in_pixels  = 14;
const Int32  Character::c_attack_height_in_pixels = 8;
const Real32 Character::c_attack_width_in_meters  = pixels_to_meters ( Character::c_attack_width_in_pixels );
const Real32 Character::c_attack_height_in_meters = pixels_to_meters ( Character::c_attack_height_in_pixels );
const Real32 Character::c_attack_time             = 0.35f;
const Real32 Character::c_cooldown_time           = 0.25f;
const Real32 Character::c_dying_time              = 1.0f;
const Int32  Character::c_fire_tick_max           = 3;
const Real32 Character::c_fire_tick_rate          = 2.0f;
const Real32 Character::c_ice_time                = 6.0f;
const Real32 Character::c_ice_decel               = 0.5f;
const Real32 Character::c_moving_walkway_accel    = 4.0f;

Bool Character::collides_with ( const Character& character )
{
     return rect_collides_with_rect ( collision_x ( ), collision_y ( ),
                                      collision_width ( ), collision_height ( ),
                                      character.collision_x ( ), character.collision_y ( ),
                                      character.collision_width ( ), character.collision_height ( ) );
}

Bool Character::attack ( )
{
     if ( !is_alive ( ) || !cooldown_watch.expired ( ) ) {
          return false;
     }

     if ( !is_blinking ( ) && !is_attacking ( ) ) {
          state_watch.reset ( Character::c_attack_time );
          state = State::attacking;
          return true;
     }

     return false;
}

Real32 Character::attack_x ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
          return position.x ( ) - c_attack_width_in_meters;
     case Direction::right:
          return position.x ( ) + dimension.x ( );
     case Direction::up:
     case Direction::down:
          return position.x ( );
     }

     return 0.0f;
}

Real32 Character::attack_y ( ) const
{
     switch ( facing ) {
     default:
          ASSERT ( 0 );
     case Direction::left:
     case Direction::right:
          return position.y ( );
     case Direction::up:
          return position.y ( ) + dimension.y ( );
     case Direction::down:
          return position.y ( ) - c_attack_height_in_meters;
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

     return 0;
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

     return 0;
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
     if ( !is_alive ( ) ) {
          return;
     }

     health -= amount;

     damage_pushed = push;

     cooldown_watch.reset ( 0.0f );
     damage_watch.reset ( Character::c_damage_time );
     state_watch.reset ( Character::c_blink_time );
     state = State::blinking;

     process_health ( );
}

Void Character::heal ( Int32 amount )
{
     health += amount;

     process_health ( );
}

Void Character::process_health ( )
{
     if ( health > max_health ) {
          health = max_health;
     }

     if ( health <= 0 ) {
          life_state = LifeState::dying;
          cooldown_watch.reset ( Character::c_dying_time );
     }
}

Void Character::block ( )
{
     if ( !is_alive ( ) ) {
          return;
     }

     if ( state == State::idle ) {
          state = State::blocking;
     }
}

Void Character::effect_with_element ( Element element )
{
     effected_by_element = transition_element ( effected_by_element, element );

     switch ( element ) {
     default:
          break;
     case Element::fire:
          element_watch.reset ( c_fire_tick_rate );
          fire_tick_count = 0;
          break;
     case Element::ice:
          element_watch.reset ( c_ice_time );
          break;
     }
}

Bool Character::in_tile ( Int32 x, Int32 y ) const
{
     float tile_left   = pixels_to_meters ( x * Map::c_tile_dimension_in_pixels );
     float tile_bottom = pixels_to_meters ( y * Map::c_tile_dimension_in_pixels );

     return rect_collides_with_rect ( collision_x ( ), collision_y ( ), collision_width ( ), collision_height ( ),
                                      tile_left, tile_bottom,
                                      Map::c_tile_dimension_in_meters, Map::c_tile_dimension_in_meters );
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

     acceleration.normalize ( );
     acceleration *= walk_acceleration;

     // if we are dying, wait til our cooldown expires
     if ( is_dying ( ) ) {
          if ( cooldown_watch.expired ( ) ) {
               life_state = Entity::LifeState::dead;
          }
     }

     // logic based on current state
     switch ( state ) {
     case State::blinking:
     {
          Vector damage_velocity;

          damage_watch.tick ( time_delta );

          if ( !damage_watch.expired ( ) && knockbackable ) {
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
               if ( life_state != LifeState::dead ) {
                    state = State::idle;
               }
          }
     } break;
     case State::attacking:
          if ( state_watch.expired ( ) ) {
               cooldown_watch.reset ( Character::c_cooldown_time );
               state = State::idle;
          }

          break;
     case State::pushing:
          // must keep pushing to stay in that state
          state = State::idle;
          break;
     case State::blocking:
          state = State::idle;
          acceleration.zero ( );
          velocity.zero ( );
          break;
     default:
          break;
     }

     switch ( effected_by_element ) {
     default:
          break;
     case Element::fire:
          element_watch.tick ( time_delta );
          break;
     case Element::ice:
          element_watch.tick ( time_delta );
          acceleration.zero ( );

          if ( element_watch.expired ( ) ) {
               effected_by_element = Element::none;
          }
          break;
     }

     switch ( on_moving_walkway ) {
     default:
          break;
     case Direction::left:
          acceleration += Vector ( -1.0f, 0.0f ) * c_moving_walkway_accel;
          break;
     case Direction::up:
          acceleration += Vector ( 0.0f, 1.0f ) * c_moving_walkway_accel;
          break;
     case Direction::right:
          acceleration += Vector ( 1.0f, 0.0f ) * c_moving_walkway_accel;
          break;
     case Direction::down:
          acceleration += Vector ( 0.0f, -1.0f ) * c_moving_walkway_accel;
          break;
     }

     // TEMPORARY, slow character down
     if ( on_ice ) {
          acceleration += velocity * -( c_ice_decel * deceleration_scale );
     } else {
          acceleration += velocity * -deceleration_scale;
     }

     Vector change_in_position = ( velocity * time_delta ) +
                                 ( acceleration * ( 0.5f * square ( time_delta ) ) );

     velocity = ( acceleration * time_delta ) + velocity;

     if ( effected_by_element != Element::ice ) {
          // Note: move along the current animation type as long as
          //       the character is not frozen
          if ( constant_animation ) {
               walk_tracker += time_delta;

               if ( walk_tracker > walk_frame_rate ) {
                    walk_frame += walk_frame_change;
                    if ( walk_frame <= 0 ||
                         walk_frame >= ( walk_frame_count - 1 ) ) {
                         walk_frame_change = -walk_frame_change;
                    }
                    walk_tracker -= walk_frame_rate;
               }
          } else {
               walk_tracker += velocity.length ( );

               if ( walk_tracker > walk_frame_rate ) {
                    walk_frame += walk_frame_change;
                    if ( walk_frame <= 0 ||
                         walk_frame >= ( walk_frame_count - 1 ) ) {
                         walk_frame_change = -walk_frame_change;
                    }
                    walk_tracker -= walk_frame_rate;
               }
          }
     }

     Real32 half_width  = collision_width ( ) * 0.5f;
     Real32 half_height = collision_height ( ) * 0.5f;

     Vector center { collision_x ( ) + half_width,
                     collision_y ( ) + half_height };

     Location old_tile = Map::vector_to_location ( center );

     Int32 center_tile_x = meters_to_pixels ( center.x ( ) ) / Map::c_tile_dimension_in_pixels;
     Int32 center_tile_y = meters_to_pixels ( center.y ( ) ) / Map::c_tile_dimension_in_pixels;

     // Note: These ignore tiles try to solve the problem of a player being on top of a pop up block
     Location ignore_tile ( center_tile_x, center_tile_y );

     switch ( facing ) {
     default:
          break;
     case Direction::left:
          ignore_tile.x++;
          break;
     case Direction::up:
          ignore_tile.y--;
          break;
     case Direction::right:
          ignore_tile.x--;
          break;
     case Direction::down:
          ignore_tile.y++;
          break;
     }

     // TODO: move tiles based on width
     Int32 min_check_tile_x = center_tile_x - 1;
     Int32 min_check_tile_y = center_tile_y - 1;
     Int32 max_check_tile_x = center_tile_x + 1;
     Int32 max_check_tile_y = center_tile_y + 1;

     // Note: Bounds are outsize the map, we treat the outsides of the map as solid so that enemies
     //       collided against the edges if they are open
     CLAMP ( min_check_tile_x, -1, map.width  ( ) );
     CLAMP ( min_check_tile_y, -1, map.height ( ) );
     CLAMP ( max_check_tile_x, -1, map.width  ( ) );
     CLAMP ( max_check_tile_y, -1, map.height ( ) );

     Real32 time_remaining = 1.0f;

     collided_last_frame = Direction::count;

     for ( int i = 0; i < 4 && time_remaining > 0.0f; ++i ) {
          // Note: When I added code to exit early when we collided against a wall,
          //       the player could clip through walls at a certain point on each tile
          Vector wall_normal;
          Real32 closest_time_intersection = time_remaining;
          Direction push_direction = Direction::count;
          Location tile;

          // loop over tile area
          for ( tile.y = min_check_tile_y; tile.y <= max_check_tile_y; ++tile.y ) {
               for ( tile.x = min_check_tile_x; tile.x <= max_check_tile_x; ++tile.x ) {

                    if ( tile == ignore_tile ) {
                         continue;
                    }

                    if ( map.tile_location_is_valid ( tile ) && !map.get_tile_location_solid ( tile ) ) {
                         if ( interactives.is_walkable ( tile, facing ) ) {
                              Auto& interactive = interactives.get_from_tile ( tile );

                              if ( interactive.type == Interactive::Type::exit ) {
                                   if ( !collides_with_exits ) {
                                        continue;
                                   }
                              } else {
                                   continue;
                              }
                         } else {
                              if ( flies ) {
                                   continue;
                              }
                         }
                    }

                    Real32 left   = pixels_to_meters ( tile.x * Map::c_tile_dimension_in_pixels );
                    Real32 right  = left + Map::c_tile_dimension_in_meters;
                    Real32 bottom = pixels_to_meters ( tile.y * Map::c_tile_dimension_in_pixels );
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
                         collided_last_frame = Direction::right;

                         if ( map.tile_location_is_valid ( tile ) &&
                              !interactives.is_walkable ( tile, facing ) ) {
                              push_direction = Direction::right;
                         }
                    }

                    if ( check_wall ( right, bottom, top, center.x ( ), change_in_position.x ( ),
                                      center.y ( ), change_in_position.y ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { 1.0f, 0.0f };
                         collided_last_frame = Direction::left;

                         if ( map.tile_location_is_valid ( tile ) &&
                              !interactives.is_walkable ( tile, facing ) ) {
                              push_direction = Direction::left;
                         }
                    }

                    if ( check_wall ( bottom, left, right, center.y ( ), change_in_position.y ( ),
                                      center.x ( ), change_in_position.x ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { 0.0f, -1.0f };
                         collided_last_frame = Direction::up;

                         if ( map.tile_location_is_valid ( tile ) &&
                              !interactives.is_walkable ( tile, facing ) ) {
                              push_direction = Direction::up;
                         }
                    }

                    if ( check_wall ( top, left, right, center.y ( ), change_in_position.y ( ),
                                      center.x ( ), change_in_position.x ( ),
                                      &closest_time_intersection ) ) {
                         wall_normal = { 0.0f, 1.0f };
                         collided_last_frame = Direction::down;

                         if ( map.tile_location_is_valid ( tile ) &&
                              !interactives.is_walkable ( tile, facing ) ) {
                              push_direction = Direction::down;
                         }
                    }
               }
          }

          // push any interactives we are colliding with
          if ( push_direction != Direction::count && state == Character::State::idle ) {
               state = pushing;
          }

          // update our physics quantities
          position += ( change_in_position * ( closest_time_intersection - 0.01f ) );
          center += ( change_in_position * ( closest_time_intersection - 0.01f ) );
          velocity -= ( wall_normal * velocity.inner_product ( wall_normal ) );
          change_in_position -= ( wall_normal * change_in_position.inner_product ( wall_normal ) );

          time_remaining -= closest_time_intersection;
     }

     Location new_tile = Map::vector_to_location ( collision_center ( ) );

     if ( new_tile != old_tile && !flies ) {
          // TODO: what about non-collides_with_interactives and portals?
          if ( map.tile_location_is_valid ( old_tile ) && map.tile_location_is_valid ( old_tile ) ) {
               interactives.character_leave ( old_tile, *this );
               interactives.character_enter ( new_tile, *this );
          }
     }

     acceleration.zero ( );
}

Void Character::walk ( Direction dir )
{
     if ( !is_alive ( ) ) {
          return;
     }

     switch ( dir ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          acceleration += Vector ( -1.0f, 0.0f );
          break;
     case Direction::up:
          acceleration += Vector ( 0.0f, 1.0f );
          break;
     case Direction::right:
          acceleration += Vector ( 1.0, 0.0f );
          break;
     case Direction::down:
          acceleration += Vector ( 0.0f, -1.0f );
          break;
     }

     change_facing ( dir );
}

Void Character::set_collision_center ( Real32 x, Real32 y )
{
     Real32 x_offset = collision_center_x ( ) - position.x ( );
     Real32 y_offset = collision_center_y ( ) - position.y ( );

     position = Vector{ x - x_offset, y - y_offset };
}

Void Character::change_facing ( Direction dir )
{
     if ( effected_by_element != Element::ice ) {
          facing = dir;
     }
}

