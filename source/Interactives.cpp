#include "Interactives.hpp"
#include "Enemy.hpp"
#include "Log.hpp"

using namespace bryte;

static const Real32 c_lever_cooldown     = 0.75f;
static const Real32 c_lean_on_block_time = 0.6f;

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

Void Interactives::update ( Real32 time_delta )
{
     Int32 count = m_width * m_height;

     for ( Int32 i = 0; i < count; ++i ) {
          m_interactives[ i ].update ( time_delta );
     }
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
                    if ( interactive.interactive_torch.on ) {
                         map.illuminate ( x * Map::c_tile_dimension_in_pixels,
                                          y * Map::c_tile_dimension_in_pixels,
                                          interactive.interactive_torch.value );
                    }
                    break;
               case Interactive::Type::pushable_torch:
                    if ( interactive.interactive_pushable_torch.torch.on ) {
                         map.illuminate ( x * Map::c_tile_dimension_in_pixels,
                                          y * Map::c_tile_dimension_in_pixels,
                                          interactive.interactive_pushable_torch.torch.value );
                    }
                    break;
               }
          }
     }
}

Void Interactives::push ( Int32 tile_x, Int32 tile_y, Direction dir, const Map& map )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     Direction result_dir = i.push ( dir, *this );

     Int32 dest_x = tile_x;
     Int32 dest_y = tile_y;

     switch ( result_dir ) {
     default:
          break;
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

     if ( dest_i.type != Interactive::Type::none ||
          map.get_coordinate_solid ( dest_x, dest_y ) ) {
          free_to_move = false;
     }

#if 0
     // TODO: put checking for characters back in when it is quicker to find them
     if ( free_to_move && player.in_tile ( dest_x, dest_y ) ) {
          free_to_move = false;
     }

     if ( free_to_move ) {
          for ( Int32 i = 0; i < enemy_count; ++i ) {
               if ( enemies [ i ].in_tile ( dest_x, dest_y ) ) {
                    free_to_move = false;
                    break;
               }
          }
     }
#endif

     if ( free_to_move ) {
          dest_i = i;
          i.type = Interactive::Type::none;
     }
}

Void Interactives::activate ( Int32 tile_x, Int32 tile_y )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.activate ( *this );
}

Void Interactives::light ( Int32 tile_x, Int32 tile_y, Uint8 light )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.light ( light, *this );
}

Void Interactives::enter ( Int32 tile_x, Int32 tile_y )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.enter ( *this );
}

Void Interactives::leave ( Int32 tile_x, Int32 tile_y )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.leave ( *this );
}

Bool Interactive::is_solid ( ) const
{
     if ( underneath.type == UnderneathInteractive::Type::popup_block &&
          underneath.underneath_popup_block.up ) {
          return true;
     }

     switch ( type ) {
     default:
          break;
     case lever:
     case torch:
     case pushable_block:
     case pushable_torch:
          return true;
     case exit:
          return interactive_exit.state != Exit::State::open;
     }

     return false;
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
     case Type::light_detector:
          interactive_light_detector.reset ( );
          break;
     }

     underneath.reset ( );
}

Void Interactive::activate ( Interactives& interactives )
{
     switch ( type ) {
     default:
          if ( underneath.type == UnderneathInteractive::Type::popup_block ) {
               underneath.underneath_popup_block.up = !underneath.underneath_popup_block.up;
          }
          break;
     case Type::none:
          break;
     case Type::exit:
          interactive_exit.activate ( );
          break;
     case Type::lever:
          interactive_lever.activate ( interactives );
          break;
     case Type::torch:
          interactive_torch.activate ( );
          break;
     case Type::pushable_torch:
          interactive_pushable_torch.activate ( );
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

     return Direction::count;
}

Void Interactive::light ( Uint8 light, Interactives& interactives )
{
     switch ( type ) {
     default:
          break;
     case Type::light_detector:
          interactive_light_detector.light ( light, interactives );
          break;
     }
}

Void Interactive::enter ( Interactives& interactives )
{
     if ( type == Type::none && underneath.type == UnderneathInteractive::Type::pressure_plate ) {
          Auto& pressure_plate = underneath.underneath_pressure_plate;

          pressure_plate.entered = true;

          Auto& interactive = interactives.get_from_tile ( pressure_plate.activate_coordinate_x,
                                                           pressure_plate.activate_coordinate_y );

          interactive.activate ( interactives );
     }
}

Void Interactive::leave ( Interactives& interactives )
{
     if ( type == Type::none && underneath.type == UnderneathInteractive::Type::pressure_plate ) {
          Auto& pressure_plate = underneath.underneath_pressure_plate;

          pressure_plate.entered = false;

          Auto& interactive = interactives.get_from_tile ( pressure_plate.activate_coordinate_x,
                                                           pressure_plate.activate_coordinate_y );

          interactive.activate ( interactives );
     }
}

Void Interactive::update ( Real32 time_delta )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::none:
     case Type::exit:
     case Type::torch:
     case Type::light_detector:
          break;
     case Type::lever:
          interactive_lever.update ( time_delta );
          break;
     case Type::pushable_block:
          interactive_pushable_block.update ( time_delta );
          break;
     case Type::pushable_torch:
          interactive_pushable_torch.update ( time_delta );
          break;
     }
}

Void Lever::reset ( )
{
     cooldown_watch.reset ( 0.0f );
     on                    = false;
     activate_coordinate_x = 0;
     activate_coordinate_y = 0;
}

Void Lever::update ( Real32 time_delta )
{
     cooldown_watch.tick ( time_delta );
}

Void Lever::activate ( Interactives& interactives )
{
     if ( !cooldown_watch.expired ( ) ) {
          return;
     }

     Auto& interactive = interactives.get_from_tile ( activate_coordinate_x, activate_coordinate_y );
     interactive.activate ( interactives );

     // toggle
     on = !on;

     // reset the stopwatch
     cooldown_watch.reset ( c_lever_cooldown );
}

Void PushableBlock::reset ( )
{
     state                = idle;
     cooldown_watch.reset ( 0.0f );
     restricted_direction = Direction::count;
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
          // restrict direction can be pushed if desired
          if ( restricted_direction != Direction::count &&
               restricted_direction != direction ) {
               break;
          }

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

Void Exit::reset ( )
{
     direction    = Direction::up;
     state        = State::open;
     map_index    = 0;
     exit_index_x = 0;
     exit_index_y = 0;
}

Void Exit::activate ( )
{
     switch ( state ) {
     default:
          ASSERT ( 0 );
          break;
     case State::open:
          state = State::closed;
          break;
     case State::closed:
     case State::locked:
          state = State::open;
          break;
     }
}

Void Torch::reset ( )
{
     on    = true;
     value = 255;
}

Void Torch::activate ( )
{
     on = !on;
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

Void PushableTorch::activate ( )
{
     torch.activate ( );
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
