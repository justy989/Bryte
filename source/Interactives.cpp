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
     case Type::portal:
          underneath_portal.reset ( );
          break;
     }
}

Interactive& Interactives::get_from_tile ( Int32 tile_x, Int32 tile_y )
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     return m_interactives [ ( tile_y * m_width ) + tile_x ];
}

const Interactive& Interactives::cget_from_tile ( Int32 tile_x, Int32 tile_y ) const
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     return m_interactives [ ( tile_y * m_width ) + tile_x ];
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

Interactive& Interactives::add ( Interactive::Type type, Int32 tile_x, Int32 tile_y )
{
     Auto& i = get_from_tile ( tile_x, tile_y );

     i.type = type;
     i.reset ( );

     return i;
}

Void Interactives::contribute_light ( Map& map )
{
     for ( Int32 y = 0; y < height ( ); ++y ) {
          for ( Int32 x = 0; x < width ( ); ++x ) {
               Auto& interactive = get_from_tile ( x, y );

               switch ( interactive.type ) {
               default:
                    break;
               case Interactive::Type::torch:
                    if ( interactive.interactive_torch.element == Element::fire ) {
                         map.illuminate ( x * Map::c_tile_dimension_in_pixels,
                                          y * Map::c_tile_dimension_in_pixels,
                                          interactive.interactive_torch.value );
                    }
                    break;
               case Interactive::Type::pushable_torch:
                    if ( interactive.interactive_pushable_torch.torch.element == Element::fire ) {
                         map.illuminate ( x * Map::c_tile_dimension_in_pixels,
                                          y * Map::c_tile_dimension_in_pixels,
                                          interactive.interactive_pushable_torch.torch.value );
                    }
                    break;
               }
          }
     }
}

Bool Interactives::push ( Int32 tile_x, Int32 tile_y, Direction dir, const Map& map )
{
     Interactive& i = get_destination_interactive ( tile_x, tile_y, &tile_x, &tile_y );

     Direction result_dir = i.push ( dir, *this );

     Int32 dest_x = tile_x;
     Int32 dest_y = tile_y;

     switch ( result_dir ) {
     default:
          return false;
     case Direction::left:
          dest_x--;
          break;
     case Direction::right:
          dest_x++;
          break;
     case Direction::up:
          dest_y++;
          break;
     case Direction::down:
          dest_y--;
          break;
     }

     Bool free_to_move = true;

     Interactive& dest_i = get_from_tile ( dest_x, dest_y );

     if ( ( !is_walkable ( dest_x, dest_y ) &&
            dest_i.underneath.type != UnderneathInteractive::Type::hole ) ||
          map.get_coordinate_solid ( dest_x, dest_y ) ) {
          free_to_move = false;
     }

     if ( free_to_move ) {
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

Bool Interactives::activate ( Int32 tile_x, Int32 tile_y )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     return i.activate ( *this );
}

Void Interactives::explode ( Int32 tile_x, Int32 tile_y )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.explode ( *this );
}

Void Interactives::light ( Int32 tile_x, Int32 tile_y, Uint8 light )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.light ( light, *this );
}

Void Interactives::character_enter ( Int32 tile_x, Int32 tile_y, Character& character )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.character_enter ( character.facing, *this, character );
}

Void Interactives::character_leave ( Int32 tile_x, Int32 tile_y, Character& character )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.character_leave ( character.facing, *this, character );
}

Void Interactives::projectile_enter ( Int32 tile_x, Int32 tile_y, Projectile& projectile )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.projectile_enter ( projectile.facing, *this, projectile );
}

Void Interactives::spread_ice ( Int32 tile_x, Int32 tile_y, const Map& map, bool clear )
{
     const Int32 c_tile_radius = 1;

     Int32 min_tile_x  = tile_x - c_tile_radius;
     Int32 max_tile_x  = tile_x + c_tile_radius;
     Int32 min_tile_y  = tile_y - c_tile_radius;
     Int32 max_tile_y  = tile_y + c_tile_radius;

     CLAMP ( min_tile_x, 0, m_width - 1 );
     CLAMP ( max_tile_x, 0, m_width - 1 );
     CLAMP ( min_tile_y, 0, m_height - 1 );
     CLAMP ( max_tile_y, 0, m_height - 1 );

     if ( clear ) {
          for ( Int32 y = min_tile_y; y <= max_tile_y; ++y ) {
               for ( Int32 x = min_tile_x; x <= max_tile_x; ++x ) {
                    Auto& interactive = get_from_tile ( x, y );

                    if ( map.get_coordinate_solid ( x, y ) ) {
                         continue;
                    }

                    if ( interactive.underneath.type == UnderneathInteractive::Type::ice ) {
                         interactive.underneath.type = UnderneathInteractive::Type::none;
                    } else if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector ) {
                         Auto& detector = interactive.underneath.underneath_ice_detector;
                         if ( detector.detected ) {
                              detector.detected = !detector.detected;
                              activate ( detector.activate_coordinate_x, detector.activate_coordinate_y );
                         }
                    }
               }
          }
     } else {
          for ( Int32 y = min_tile_y; y <= max_tile_y; ++y ) {
               for ( Int32 x = min_tile_x; x <= max_tile_x; ++x ) {
                    Auto& interactive = get_from_tile ( x, y );

                    if ( map.get_coordinate_solid ( x, y ) ) {
                         continue;
                    }

                    if ( interactive.underneath.type == UnderneathInteractive::Type::none ) {
                         interactive.underneath.type = UnderneathInteractive::Type::ice;
                         interactive.underneath.underneath_ice.force_dir = Direction::count;
                    } else if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector ) {
                         Auto& detector = interactive.underneath.underneath_ice_detector;
                         if ( !detector.detected ) {
                              detector.detected = !detector.detected;
                              activate ( detector.activate_coordinate_x, detector.activate_coordinate_y );
                         }
                    }
               }
          }
     }
}

Bool Interactives::check_portal_walkability ( Int32 start_tile_x, Int32 start_tile_y,
                                              Int32 dest_tile_x, Int32 dest_tile_y ) const
{
     const Auto& interactive = cget_from_tile ( dest_tile_x, dest_tile_y );

     if ( interactive.underneath.type == UnderneathInteractive::Type::portal ) {

          // quit early if something is on top of the portal
          if ( interactive.type ) {
               return false;
          }

          Auto& portal = interactive.underneath.underneath_portal;

          // If we loop back around to our starting point, then we are walkable
          if ( portal.destination_x == start_tile_x && portal.destination_y == start_tile_y ) {
               return true;
          }

          // NOTE: recursively search portals for a destination that is not a portal
          return check_portal_walkability ( start_tile_x, start_tile_y,
                                            portal.destination_x, portal.destination_y );
     } else {
          return is_walkable ( dest_tile_x, dest_tile_y );
     }

     return true;
}

Interactive& Interactives::get_destination_interactive ( Int32 start_tile_x, Int32 start_tile_y,
                                                         Int32* dest_tile_x, Int32* dest_tile_y )
{
     Auto& interactive = get_from_tile ( *dest_tile_x, *dest_tile_y );

     // NOTE: This method is very similar to check_portal_walkability
     if ( interactive.underneath.type == UnderneathInteractive::Type::portal ) {
          if ( interactive.type ) {
               return interactive;
          }

          Auto& portal = interactive.underneath.underneath_portal;
          if ( portal.destination_x == start_tile_x && portal.destination_y == start_tile_y ) {
               return interactive;
          }

          *dest_tile_x = portal.destination_x;
          *dest_tile_y = portal.destination_y;

          return get_destination_interactive ( start_tile_x, start_tile_y,
                                               dest_tile_x, dest_tile_y );
     }

     return interactive;
}

Bool Interactives::is_walkable ( Int32 tile_x, Int32 tile_y ) const
{
     const Auto& interactive = cget_from_tile ( tile_x, tile_y );

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
     case UnderneathInteractive::Type::portal:
     {
          Auto& portal = interactive.underneath.underneath_portal;

          if ( !check_portal_walkability ( tile_x, tile_y,
                                           portal.destination_x, portal.destination_y ) ) {
               return false;
          }
     } break;
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
     }

     return true;
}

Bool Interactives::is_flyable ( Int32 tile_x, Int32 tile_y ) const
{
     const Auto& interactive = cget_from_tile ( tile_x, tile_y );

     switch ( interactive.type ) {
     default:
          break;
     case Interactive::Type::lever:
     case Interactive::Type::exit:
          return false;
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
     }

     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::portal:
          return underneath.underneath_portal.activate ( );
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
     }

     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::portal:
          return underneath.underneath_portal.push ( direction, interactives );
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

Void Interactive::character_enter ( Direction from, Interactives& interactives, Character& character )
{
     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::pressure_plate:
     {
          Auto& pressure_plate = underneath.underneath_pressure_plate;
          pressure_plate.entered = true;
          Auto& interactive = interactives.get_from_tile ( pressure_plate.activate_coordinate_x,
                                                           pressure_plate.activate_coordinate_y );
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
     case UnderneathInteractive::Type::portal:
          if ( underneath.underneath_portal.on ) {
               Vector dst = Map::coordinates_to_vector ( underneath.underneath_portal.destination_x,
                                                         underneath.underneath_portal.destination_y );
               dst += Vector { Map::c_tile_dimension_in_meters * 0.5f,
                               Map::c_tile_dimension_in_meters * 0.5f };
               character.set_collision_center ( dst.x ( ), dst.y ( ) );
          }
          break;
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

          Auto& interactive = interactives.get_from_tile ( pressure_plate.activate_coordinate_x,
                                                           pressure_plate.activate_coordinate_y );

          interactive.activate ( interactives );
     } break;
     case UnderneathInteractive::ice:
          character.on_ice = false;
          break;
     case UnderneathInteractive::moving_walkway:
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

          Auto& interactive = interactives.get_from_tile ( pressure_plate.activate_coordinate_x,
                                                           pressure_plate.activate_coordinate_y );

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
     case UnderneathInteractive::portal:
     {
          if ( underneath.underneath_portal.on ) {
               Int32 dest_x = underneath.underneath_portal.destination_x;
               Int32 dest_y = underneath.underneath_portal.destination_y;

               Auto& dest_interactive = interactives.get_from_tile ( dest_x, dest_y );

               // only move it if something isn't already there
               if ( dest_interactive.type == Interactive::Type::none ) {
                    // copy just the Interactive portion, not the underneath portion, and only
                    // support types that can move
                    switch ( type ) {
                    default:
                         return;
                    case Type::pushable_block:
                         dest_interactive.type = type;
                         dest_interactive.interactive_pushable_block = interactive_pushable_block;
                         break;
                    case Type::pushable_torch:
                         dest_interactive.type = type;
                         dest_interactive.interactive_pushable_torch = interactive_pushable_torch;
                         break;
                    }

                    // clear what is on us
                    type = Interactive::Type::none;
               }
          }
     } break;
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

          Auto& interactive = interactives.get_from_tile ( pressure_plate.activate_coordinate_x,
                                                           pressure_plate.activate_coordinate_y );

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

     switch ( underneath.type ) {
     default:
          break;
     case UnderneathInteractive::Type::portal:
     {
          if ( underneath.underneath_portal.on ) {
               Vector dst = Map::coordinates_to_vector ( underneath.underneath_portal.destination_x,
                                                         underneath.underneath_portal.destination_y );
               projectile.position = dst;
          }
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
               Auto& interactive = interactives.get_from_tile ( activate_coordinate_x, activate_coordinate_y );
               interactive.activate ( interactives );
               state = State::on;
          }
     } break;
     case State::changing_off:
     {
          cooldown_watch.tick ( time_delta );

          if ( cooldown_watch.expired ( ) ) {
               Auto& interactive = interactives.get_from_tile ( activate_coordinate_x, activate_coordinate_y );
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
               Auto& interactive = interactives.get_from_tile ( activate_coordinate_x, activate_coordinate_y );
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

               interactives.activate ( activate_coordinate_x, activate_coordinate_y );
          } else if ( value < c_bryte_value && !below_value ) {
               below_value = true;

               interactives.activate ( activate_coordinate_x, activate_coordinate_y );
          }
          break;
     case Type::dark:
          if ( value <= c_dark_value && !below_value ) {
               below_value = true;

               interactives.activate ( activate_coordinate_x, activate_coordinate_y );
          } else if ( value > c_dark_value && below_value ) {
               below_value = false;

               interactives.activate ( activate_coordinate_x, activate_coordinate_y );
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
     on = false;
     destination_x = 0;
     destination_y = 0;
}

Direction Portal::push ( Direction direction, Interactives& interactives )
{
     // just pass along the push to the target
     Auto& interactive = interactives.get_from_tile ( destination_x, destination_y );

     return interactive.push ( direction, interactives );
}

Bool Portal::activate ( )
{
     on = !on;

     return true;
}

