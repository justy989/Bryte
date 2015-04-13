#include "Interactives.hpp"
#include "Projectile.hpp"
#include "Enemy.hpp"
#include "Log.hpp"

using namespace bryte;

static const Real32 c_lever_cooldown     = 0.75f;
static const Real32 c_exit_change_time   = 0.5f;
static const Real32 c_lean_on_block_time = 0.3f;

const Real32 Turret::c_shoot_interval = 3.0f;

Void UnderneathInteractive::reset ( )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case none:
          break;
     case pressure_plate:
          underneath_popup_block.up = false;
          break;
     case popup_block:
          underneath_pressure_plate.activate_coordinate_x = 0;
          underneath_pressure_plate.activate_coordinate_y = 0;
          break;
     case ice:
          underneath_ice.reset ( );
          break;
     case moving_walkway:
          underneath_moving_walkway.reset ( );
          break;
     case light_detector:
          underneath_light_detector.reset ( );
          break;
     case ice_detector:
          underneath_ice_detector.reset ( );
          break;
     case hole:
          underneath_hole.filled = false;
          break;
     }
}

Interactive& Interactives::get_from_tile ( const Location& tile )
{
     ASSERT ( tile.x >= 0 && tile.x < m_width );
     ASSERT ( tile.y >= 0 && tile.y < m_height );

     return m_interactives [ ( tile.y * m_width ) + tile.x ];
}

const Interactive& Interactives::cget_from_tile ( const Location& tile ) const
{
     ASSERT ( tile.x >= 0 && tile.x < m_width );
     ASSERT ( tile.y >= 0 && tile.y < m_height );

     return m_interactives [ ( tile.y * m_width ) + tile.x ];
}

Void Interactives::reset ( Int32 width, Int32 height )
{
     m_width  = width;
     m_height = height;

     Int32 count = m_width * m_height;

     for ( int i = 0; i < count; ++i ) {
          m_interactives [ i ].type = Interactive::Type::none;
          m_interactives [ i ].underneath.type = UnderneathInteractive::Type::none;
     }
}

Interactive& Interactives::add ( Interactive::Type type, const Location& tile )
{
     Auto& i = get_from_tile ( tile );

     // TODO: reset underneath as well
     i.type = type;
     i.reset ( );

     return i;
}

Void Interactives::contribute_light ( Map& map )
{
     for ( Location tile; tile.y < height ( ); ++tile.y ) {
          for ( tile.x = 0; tile.x < width ( ); ++tile.x ) {
               Auto& interactive = get_from_tile ( tile );

               switch ( interactive.type ) {
               default:
                    break;
               case Interactive::Type::torch:
                    if ( interactive.interactive_torch.element == Element::fire ) {
                         map.illuminate ( tile, interactive.interactive_torch.value );
                    }
                    break;
               case Interactive::Type::pushable_torch:
                    if ( interactive.interactive_pushable_torch.torch.element == Element::fire ) {
                         map.illuminate ( tile, interactive.interactive_pushable_torch.torch.value );
                    }
                    break;
               }
          }
     }
}

Bool Interactives::push ( const Location& tile, Direction dir, const Map& map )
{
     Location push_tile ( tile );
     Interactive& i_portal = get_from_tile ( tile );

     if ( i_portal.type == Interactive::Type::portal ) {
          get_portal_destination ( &push_tile, dir );
     }

     Interactive& i = get_from_tile ( push_tile );
     Direction result_dir = i.push ( dir, *this );
     Location dest_tile ( tile );

     switch ( result_dir ) {
     default:
          return false;
     case Direction::left:
          dest_tile.x--;
          break;
     case Direction::right:
          dest_tile.x++;
          break;
     case Direction::up:
          dest_tile.y++;
          break;
     case Direction::down:
          dest_tile.y--;
          break;
     }

     Interactive& dest_i = get_from_tile ( dest_tile );

     if ( ( !is_walkable ( dest_tile, dir ) &&
            dest_i.underneath.type != UnderneathInteractive::Type::hole ) ||
            map.get_tile_location_solid ( dest_tile ) ) {
          // pass
     } else if ( dest_i.type == Interactive::Type::portal ) {
          Location portal_dest_tile ( dest_i.interactive_portal.destination_x,
                                      dest_i.interactive_portal.destination_y );

          move_tile_location ( &portal_dest_tile, result_dir );

          if ( is_walkable ( portal_dest_tile, result_dir ) ) {
               Interactive& portal_dest_i = get_from_tile ( dest_tile );
               portal_dest_i = i;
               i.type = Interactive::Type::none;
               i.interactive_leave ( result_dir, *this );
               portal_dest_i.interactive_enter ( result_dir, *this );
          }
     } else {
          // save underneath the dest
          UnderneathInteractive save_underneath = dest_i.underneath;

          // copy to the dest
          dest_i = i;

          // restore the underneath
          dest_i.underneath = save_underneath;

          // clear the source
          i.type = Interactive::Type::none;

          // enter and leave both tiles
          i.interactive_leave ( result_dir, *this );
          dest_i.interactive_enter ( result_dir, *this );

          return true;
     }

     // stop ice from pushing if we can't go to the next square
     if ( i.underneath.type == UnderneathInteractive::Type::ice &&
          i.underneath.underneath_ice.force_dir != Direction::count ) {
          i.underneath.underneath_ice.force_dir = Direction::count;
     }

     if ( i.underneath.type == UnderneathInteractive::Type::ice_detector &&
          i.underneath.underneath_ice_detector.force_dir != Direction::count ) {
          i.underneath.underneath_ice_detector.force_dir = Direction::count;
     }

     return false;
}

Bool Interactives::activate ( const Location& tile )
{
     Interactive& i = get_from_tile ( tile );

     return i.activate ( *this );
}

Void Interactives::explode ( const Location& tile )
{
     Interactive& i = get_from_tile ( tile );

     i.explode ( *this );
}

Void Interactives::light ( const Location& tile, Uint8 light )
{
     Interactive& i = get_from_tile ( tile );

     i.light ( light, *this );
}

Void Interactives::attack ( const Location& tile )
{
     Interactive& i = get_from_tile ( tile );

     i.attack ( *this );
}

Void Interactives::character_enter ( const Location& tile, Character& character )
{
     Interactive& i = get_from_tile ( tile );

     i.character_enter ( character.facing, *this, character );
}

Void Interactives::character_leave ( const Location& tile, Character& character )
{
     Interactive& i = get_from_tile ( tile );

     i.character_leave ( character.facing, *this, character );
}

Void Interactives::projectile_enter ( const Location& tile, Projectile& projectile )
{
     Interactive& i = get_from_tile ( tile );

     i.projectile_enter ( projectile.facing, *this, projectile );
}

Void Interactives::spread_ice ( const Location& tile, const Map& map, bool clear )
{
     const Int32 c_tile_radius = 1;

     Int32 min_tile_x  = tile.x - c_tile_radius;
     Int32 max_tile_x  = tile.x + c_tile_radius;
     Int32 min_tile_y  = tile.y - c_tile_radius;
     Int32 max_tile_y  = tile.y + c_tile_radius;

     CLAMP ( min_tile_x, 0, m_width - 1 );
     CLAMP ( max_tile_x, 0, m_width - 1 );
     CLAMP ( min_tile_y, 0, m_height - 1 );
     CLAMP ( max_tile_y, 0, m_height - 1 );

     if ( clear ) {
          for ( Int32 y = min_tile_y; y <= max_tile_y; ++y ) {
               for ( Int32 x = min_tile_x; x <= max_tile_x; ++x ) {
                    Location current_tile ( x, y );
                    Auto& interactive = get_from_tile ( current_tile );

                    if ( map.get_tile_location_solid ( current_tile ) ) {
                         continue;
                    }

                    if ( interactive.underneath.type == UnderneathInteractive::Type::ice ) {
                         interactive.underneath.type = UnderneathInteractive::Type::none;
                    } else if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector ) {
                         Auto& detector = interactive.underneath.underneath_ice_detector;
                         if ( detector.detected ) {
                              detector.detected = !detector.detected;
                              Location activate_tile ( detector.activate_coordinate_x, detector.activate_coordinate_y );
                              activate ( activate_tile );
                         }
                    }
               }
          }
     } else {
          for ( Int32 y = min_tile_y; y <= max_tile_y; ++y ) {
               for ( Int32 x = min_tile_x; x <= max_tile_x; ++x ) {
                    Location current_tile ( x, y );
                    Auto& interactive = get_from_tile ( current_tile );

                    if ( map.get_tile_location_solid ( current_tile ) ) {
                         continue;
                    }

                    if ( interactive.underneath.type == UnderneathInteractive::Type::none ) {
                         interactive.underneath.type = UnderneathInteractive::Type::ice;
                         interactive.underneath.underneath_ice.force_dir = Direction::count;
                    } else if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector ) {
                         Auto& detector = interactive.underneath.underneath_ice_detector;
                         if ( !detector.detected ) {
                              detector.detected = !detector.detected;
                              Location activate_tile ( detector.activate_coordinate_x, detector.activate_coordinate_y );
                              activate ( activate_tile );
                         }
                    }
               }
          }
     }
}

Bool Interactives::is_walkable ( const Location& tile, Direction dir ) const
{
     const Auto& interactive = cget_from_tile ( tile );

     switch ( interactive.underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::popup_block:
          if ( interactive.underneath.underneath_popup_block.up ) {
               return false;
          }
          break;
     case UnderneathInteractive::Type::hole:
          if ( !interactive.underneath.underneath_hole.filled ) {
               return false;
          }
          break;
     case UnderneathInteractive::Type::destructable:
          if ( !interactive.underneath.underneath_destructable.destroyed ) {
               return false;
          }
          break;
     }

     switch ( interactive.type ) {
     default:
          break;
     case Interactive::Type::lever:
     case Interactive::Type::pushable_block:
     case Interactive::Type::torch:
     case Interactive::Type::pushable_torch:
     case Interactive::Type::bombable_block:
          return false;
     case Interactive::Type::exit:
          return interactive.interactive_exit.state == Exit::State::open;
     case Interactive::Type::portal:
     {
          Auto dest_tile = tile;

          get_portal_destination ( &dest_tile, dir );

          return is_walkable ( dest_tile, dir );
     } break;
     }

     return true;
}

Bool Interactives::is_flyable ( const Location& tile ) const
{
     const Auto& interactive = cget_from_tile ( tile );

     switch ( interactive.type ) {
     default:
          break;
     case Interactive::Type::exit:
          return false;
     }

     switch ( interactive.underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::destructable:
          return false;;
     }

     return true;
}

Void Interactive::reset ( )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::none:
          break;
     case Type::exit:
          interactive_exit.reset ( );
          break;
     case Type::lever:
          interactive_lever.reset ( );
          break;
     case Type::pushable_block:
          interactive_pushable_block.reset ( );
          break;
     case Type::torch:
          interactive_torch.reset ( );
          break;
     case Type::pushable_torch:
          interactive_pushable_torch.reset ( );
          break;
     case Type::bombable_block:
          break;
     case Type::turret:
          interactive_turret.reset ( );
          break;
     case Type::portal:
          interactive_portal.reset ( );
          break;
     }

     underneath.reset ( );
}

Bool Interactive::activate ( Interactives& interactives )
{
     switch ( type ) {
     default:
          break;
     case Type::none:
          if ( underneath.type == UnderneathInteractive::Type::popup_block ) {
               underneath.underneath_popup_block.up = !underneath.underneath_popup_block.up;
          } else if ( underneath.type == UnderneathInteractive::Type::moving_walkway ) {
               underneath.underneath_moving_walkway.facing =
                    opposite_direction ( underneath.underneath_moving_walkway.facing );
          }
          break;
     case Type::exit:
          return interactive_exit.activate ( );
     case Type::lever:
          return interactive_lever.activate ( );
     case Type::torch:
          return interactive_torch.activate ( );
     case Type::pushable_torch:
          return interactive_pushable_torch.activate ( );
     case Type::turret:
          return interactive_turret.activate ( );
     case Type::portal:
          return interactive_portal.activate ( );
     }

     return false;
}

Void Interactive::explode ( Interactives& interactives )
{
     switch ( type ) {
     default:
          break;
     case Type::none:
          break;
     case Type::exit:
          break;
     case Type::lever:
          interactive_lever.activate ( );
          break;
     case Type::bombable_block:
          type = Type::none;
          break;
     }
}

Direction Interactive::push ( Direction direction, Interactives& interactives )
{
     switch ( type ) {
     default:
          break;
     case Type::pushable_block:
          return interactive_pushable_block.push ( direction, interactives );
     case Type::pushable_torch:
          return interactive_pushable_torch.push ( direction, interactives );
     case Type::portal:
          return interactive_portal.push ( direction, interactives );
     }

     return Direction::count;
}

Void Interactive::light ( Uint8 light, Interactives& interactives )
{
     // TODO: evaluate only light if not covered?
     if ( type == Interactive::Type::none &&
          underneath.type == UnderneathInteractive::Type::light_detector ) {
          underneath.underneath_light_detector.light ( light, interactives );
     }
}

Void Interactive::attack ( Interactives& interactives )
{
     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::destructable:
          underneath.underneath_destructable.destroyed = true;
          break;
     }
}

Void Interactive::character_enter ( Direction from, Interactives& interactives, Character& character )
{
     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::pressure_plate:
     {
          Auto& pressure_plate = underneath.underneath_pressure_plate;
          pressure_plate.entered = true;
          Location activate_tile ( pressure_plate.activate_coordinate_x,
                                   pressure_plate.activate_coordinate_y );
          Auto& interactive = interactives.get_from_tile ( activate_tile );
          interactive.activate ( interactives );
     } break;
     case UnderneathInteractive::Type::ice:
          character.on_ice = true;
          break;
     case UnderneathInteractive::Type::moving_walkway:
          character.on_moving_walkway = underneath.underneath_moving_walkway.facing;
          break;
     case UnderneathInteractive::Type::ice_detector:
          if ( underneath.underneath_ice_detector.detected ) {
               character.on_ice = true;
          }
          break;
     }

     switch ( type ) {
     default:
          break;
     case Interactive::Type::portal:
     {
          // TODO: compress with projectile_leave portal handling
          Location dest_tile ( interactive_portal.destination_x,
                               interactive_portal.destination_y );

          move_tile_location ( &dest_tile, from );

          Auto dest_position = Map::location_to_vector ( dest_tile );

          dest_position += Vector ( Map::c_tile_dimension_in_meters * 0.5f,
                                    Map::c_tile_dimension_in_meters * 0.5f );

          character.set_collision_center ( dest_position.x ( ), dest_position.y ( ) );
     } break;
     }
}

Void Interactive::character_leave ( Direction to, Interactives& interactives, Character& character )
{
     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::pressure_plate:
     {
          Auto& pressure_plate = underneath.underneath_pressure_plate;

          pressure_plate.entered = false;

          Location activate_tile ( pressure_plate.activate_coordinate_x,
                                   pressure_plate.activate_coordinate_y );
          Auto& interactive = interactives.get_from_tile ( activate_tile );

          interactive.activate ( interactives );
     } break;
     case UnderneathInteractive::ice:
          character.on_ice = false;
          break;
     case UnderneathInteractive::moving_walkway:
          character.on_moving_walkway = Direction::count;
          break;
     case UnderneathInteractive::ice_detector:
          character.on_ice = false;
          break;
     }
}

Void Interactive::interactive_enter ( Direction from, Interactives& interactives )
{
     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::pressure_plate:
     {
          Auto& pressure_plate = underneath.underneath_pressure_plate;

          pressure_plate.entered = true;

          Location activate_tile ( pressure_plate.activate_coordinate_x,
                                   pressure_plate.activate_coordinate_y );
          Auto& interactive = interactives.get_from_tile ( activate_tile );

          interactive.activate ( interactives );
     } break;
     case UnderneathInteractive::ice:
          underneath.underneath_ice.force_dir = from;
          break;
     case UnderneathInteractive::ice_detector:
          underneath.underneath_ice_detector.force_dir = from;
          break;
     case UnderneathInteractive::light_detector:
          underneath.underneath_light_detector.light ( 0, interactives );
          break;
     case UnderneathInteractive::hole:
          if ( !underneath.underneath_hole.filled ) {
               // TODO: make sure this is really only a pushable torch block?
               // consume the interactive
               type = Interactive::Type::none;
               reset ( );

               underneath.underneath_hole.filled = true;
          }
          break;
     }
}

Void Interactive::interactive_leave ( Direction to, Interactives& interactives )
{
     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::pressure_plate:
     {
          Auto& pressure_plate = underneath.underneath_pressure_plate;

          pressure_plate.entered = false;

          Location activate_tile ( pressure_plate.activate_coordinate_x,
                                   pressure_plate.activate_coordinate_y );
          Auto& interactive = interactives.get_from_tile ( activate_tile );

          interactive.activate ( interactives );
     } break;
     case UnderneathInteractive::Type::ice:
          underneath.underneath_ice.force_dir = Direction::count;
          break;
     case UnderneathInteractive::Type::ice_detector:
          underneath.underneath_ice_detector.force_dir = Direction::count;
          break;
     }
}

Void Interactive::projectile_enter ( Direction from, Interactives& interactives, Projectile& projectile )
{
     switch ( type ) {
     default:
          break;
     case Type::lever:
          activate ( interactives );
          break;
     case Type::torch:
          if ( projectile.type == Projectile::Type::arrow ) {
               if ( projectile.effected_by_element ) {
                    Auto& torch_element = interactive_torch.element;
                    torch_element = transition_element ( torch_element, projectile.effected_by_element );
               } else {
                    Auto& torch_element = interactive_torch.element;
                    projectile.effected_by_element = transition_element ( projectile.effected_by_element,
                                                                          torch_element );
               }
          }
          break;
     case Type::pushable_torch:
          if ( projectile.type == Projectile::Type::arrow ) {
               if ( projectile.effected_by_element ) {
                    Auto& torch_element = interactive_pushable_torch.torch.element;
                    torch_element = transition_element ( torch_element, projectile.effected_by_element );
               } else {
                    Auto& torch_element = interactive_pushable_torch.torch.element;
                    projectile.effected_by_element = transition_element ( projectile.effected_by_element,
                                                                          torch_element );
               }
          }
          break;
     }

     switch ( type ) {
     default:
          break;
     case Interactive::Type::portal:
     {
          Location dest_tile ( interactive_portal.destination_x,
                               interactive_portal.destination_y );

          move_tile_location ( &dest_tile, from );

          projectile.position = Map::location_to_vector ( dest_tile );
     } break;
     }
}

Void Interactive::update ( Real32 time_delta, Interactives& interactives )
{
     switch ( type ) {
     default:
          break;
     case Type::none:
     case Type::torch:
     case Type::lever:
          interactive_lever.update ( time_delta, interactives );
          break;
     case Type::pushable_block:
          interactive_pushable_block.update ( time_delta );
          break;
     case Type::pushable_torch:
          interactive_pushable_torch.update ( time_delta );
          break;
     case Type::exit:
          interactive_exit.update ( time_delta );
          break;
     case Type::turret:
          interactive_turret.update ( time_delta );
          break;
     }
}

Bool Lever::changing ( ) const
{
     return ( state == State::changing_on ||
              state == State::changing_off );
}

Void Lever::reset ( )
{
     cooldown_watch.reset ( 0.0f );
     state                 = State::off;
     activate_coordinate_x = 0;
     activate_coordinate_y = 0;
}

Void Lever::update ( Real32 time_delta, Interactives& interactives )
{
     switch ( state ) {
     default:
          break;
     case State::changing_on:
     {
          cooldown_watch.tick ( time_delta );

          if ( cooldown_watch.expired ( ) ) {
               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               Auto& interactive = interactives.get_from_tile ( activate_tile );
               interactive.activate ( interactives );
               state = State::on;
          }
     } break;
     case State::changing_off:
     {
          cooldown_watch.tick ( time_delta );

          if ( cooldown_watch.expired ( ) ) {
               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               Auto& interactive = interactives.get_from_tile ( activate_tile );
               interactive.activate ( interactives );
               state = State::off;
          }
     } break;
     }
}

Bool Lever::activate ( )
{
     if ( state == State::off ) {
          state = State::changing_on;
          cooldown_watch.reset ( c_lever_cooldown );
     } else if ( state == State::on ) {
          state = State::changing_off;
          cooldown_watch.reset ( c_lever_cooldown );
     }

     return true;
}

Void PushableBlock::reset ( )
{
     state                = idle;
     cooldown_watch.reset ( 0.0f );
     pushed_last_update   = false;
     one_time             = false;
}

Void PushableBlock::update ( Real32 time_delta )
{
     switch ( state) {
     default:
          ASSERT ( 0 );
          break;
     case idle:
          break;
     case leaned_on:
          if ( pushed_last_update ) {
               cooldown_watch.tick ( time_delta );
          } else {
               state = idle;
          }
          break;
     case moving:
          break;
     case solid:
          break;
     }

     pushed_last_update = false;
}

Direction PushableBlock::push ( Direction direction, Interactives& interactives )
{
     switch ( state) {
     default:
          ASSERT ( 0 );
          break;
     case idle:
          state = leaned_on;
          cooldown_watch.reset ( c_lean_on_block_time );
          pushed_last_update = true;
          break;
     case leaned_on:
          if ( cooldown_watch.expired ( ) ) {
               state = moving;
          }
          pushed_last_update = true;
          break;
     case moving:
          if ( one_time ) {
               state = solid;
          } else {
               state = idle;
          }

          if ( activate_coordinate_x || activate_coordinate_y ) {

               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               Auto& interactive = interactives.get_from_tile ( activate_tile );
               interactive.activate ( interactives );
          }

          return direction;
     case solid:
          break;
     }

     return Direction::count;
}

Bool Exit::changing ( ) const
{
     return ( state == changing_to_open ||
              state == changing_to_unlocked ||
              state == changing_to_closed ||
              state == changing_to_locked );
}

Void Exit::reset ( )
{
     direction    = Direction::up;
     state        = State::open;
     map_index    = 0;
     exit_index_x = 0;
     exit_index_y = 0;
}

Void Exit::update ( Real32 time_delta )
{
     switch ( state ) {
     default:
          break;
     case State::changing_to_open:
     case State::changing_to_unlocked:
     {
          state_watch.tick ( time_delta );

          if ( state_watch.expired ( ) ) {
               state = State::open;
          }
     } break;
     case State::changing_to_closed:
     {
          state_watch.tick ( time_delta );

          if ( state_watch.expired ( ) ) {
               state = State::closed;
          }
     } break;
     case State::changing_to_locked:
     {
          state_watch.tick ( time_delta );

          if ( state_watch.expired ( ) ) {
               state = State::locked;
          }
     } break;
     }
}

Bool Exit::activate ( )
{
     switch ( state ) {
     default:
          break;
     case State::open:
          state = State::changing_to_closed;
          state_watch.reset ( c_exit_change_time );
          break;
     case State::closed:
          state = State::changing_to_open;
          state_watch.reset ( c_exit_change_time );
          break;
     case State::locked:
          state = State::changing_to_unlocked;
          state_watch.reset ( c_exit_change_time );
          break;
     case State::changing_to_closed:
          state = State::open;
          break;
     case State::changing_to_open:
          state = State::closed;
          break;
     }

     return true;
}

Void Torch::reset ( )
{
     element = Element::fire;
     value   = 255;
}

Bool Torch::activate ( )
{
     switch ( element ) {
     default:
          ASSERT ( 0 );
          break;
     case Element::none:
          element = Element::fire;
          break;
     case Element::ice:
     case Element::fire:
          element = Element::none;
          break;
     }

     return true;
}

Void PushableTorch::reset ( )
{
     torch.reset ( );
     pushable_block.reset ( );
}

Void PushableTorch::update ( Real32 time_delta )
{
     pushable_block.update ( time_delta );
}

Bool PushableTorch::activate ( )
{
     torch.activate ( );
     return false;
}

Direction PushableTorch::push ( Direction direction, Interactives& interactives )
{
     return pushable_block.push ( direction, interactives );
}

Void LightDetector::reset ( )
{
     type                  = bryte;
     below_value           = true;
     activate_coordinate_x = 0;
     activate_coordinate_y = 0;
}

Void LightDetector::light ( Uint8 value, Interactives& interactives )
{
     switch ( type ) {
     case Type::bryte:
          if ( value >= c_bryte_value && below_value ) {
               below_value = false;

               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               interactives.activate ( activate_tile );
          } else if ( value < c_bryte_value && !below_value ) {
               below_value = true;

               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               interactives.activate ( activate_tile );
          }
          break;
     case Type::dark:
          if ( value <= c_dark_value && !below_value ) {
               below_value = true;

               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               interactives.activate ( activate_tile );
          } else if ( value > c_dark_value && below_value ) {
               below_value = false;

               Location activate_tile ( activate_coordinate_x, activate_coordinate_y );
               interactives.activate ( activate_tile );
          }
          break;
     }
}

Void Turret::reset ( )
{
     automatic = false;
     wants_to_shoot = false;
     automatic_watch.reset ( 0.0f );
     facing = Direction::left;
}

Bool Turret::activate ( )
{
     wants_to_shoot = true;

     return true;
}

Void Turret::update ( Real32 time_delta )
{
     wants_to_shoot = false;

     if ( automatic ) {
          automatic_watch.tick ( time_delta );

          if ( automatic_watch.expired ( ) ) {
               wants_to_shoot = true;
               automatic_watch.reset ( c_shoot_interval );
          }
     }
}

Void Ice::reset ( )
{
     force_dir = Direction::count;
}

Void MovingWalkway::reset ( )
{
     facing = Direction::left;
}

Void IceDetector::reset ( )
{
     detected = false;
     force_dir = Direction::count;
     activate_coordinate_x = 0;
     activate_coordinate_y = 0;
}

Void Portal::reset ( )
{
     destination_x = 0;
     destination_y = 0;
}

Direction Portal::push ( Direction direction, Interactives& interactives )
{
     Location dest_tile ( destination_x, destination_y );

     move_tile_location ( &dest_tile, direction );

     Auto& interactive = interactives.get_from_tile ( dest_tile );

     // just pass along the push to the target
     if ( interactive.type ) {
          return interactive.push ( direction, interactives );
     }

     return Direction::count;
}

Bool Portal::activate ( )
{
     return true;
}

Void Interactives::get_portal_destination ( Location* tile, Direction dir ) const
{
     ASSERT ( tile );

     get_portal_destination_impl ( *tile, tile, dir );
}

Void Interactives::get_portal_destination_impl ( const Location& start_tile,
                                                 Location* dest_tile,
                                                 Direction dir ) const
{
     ASSERT ( dest_tile );

     const Interactive& interactive = cget_from_tile ( *dest_tile );

     if ( interactive.type != Interactive::Type::portal ) {
          return;
     }

     dest_tile->x = interactive.interactive_portal.destination_x;
     dest_tile->y = interactive.interactive_portal.destination_y;

     move_tile_location ( dest_tile, dir );

     // Note: To (try to) avoid infinite recursion, exit if we ever see the first portal again
     if ( start_tile == *dest_tile ) {
          return;
     }

     const Interactive& dest_interactive = cget_from_tile ( *dest_tile );

     if ( dest_interactive.type == Interactive::Type::portal ) {
          get_portal_destination_impl ( start_tile, dest_tile, dir );
     }
}

