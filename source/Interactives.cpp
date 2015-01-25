#include "Interactives.hpp"
#include "Log.hpp"

using namespace bryte;

static const Real32 c_lever_cooldown     = 0.75f;
static const Real32 c_lean_on_block_time = 0.6f;

Interactive& Interactives::get_from_tile ( Int32 tile_x, Int32 tile_y )
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
     }
}

Interactive& Interactives::add ( Interactive::Type type, Int32 tile_x, Int32 tile_y )
{
     auto& i = get_from_tile ( tile_x, tile_y );

     i.type = type;
     i.reset ( );

     return i;
}

Void Interactives::update ( float time_delta )
{
     Int32 count = m_width * m_height;

     for ( Int32 i = 0; i < count; ++i ) {
          m_interactives[ i ].update ( time_delta );
     }
}

Void Interactives::push ( Int32 tile_x, Int32 tile_y, Direction dir, const Map& map )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     Direction result_dir = i.push ( dir );

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

     Interactive& dest_i = get_from_tile ( dest_x, dest_y );

     if ( dest_i.type == Interactive::Type::none &&
          !map.get_coordinate_solid ( dest_x, dest_y ) ) {
          dest_i = i;
          i.type = Interactive::Type::none;
     }
}

Void Interactives::activate ( Int32 tile_x, Int32 tile_y )
{
     Interactive& i = get_from_tile ( tile_x, tile_y );

     i.activate ( *this );
}

Bool Interactive::is_solid ( ) const
{
     switch ( type ) {
     default:
          break;
     case pushable_block:
          return true;
     case exit:
          return interactive_exit.state != Exit::State::open;
     case pushable_torch:
          return true;
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
     }
}

Void Interactive::activate ( Interactives& interactives )
{
     switch ( type ) {
     default:
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

Direction Interactive::push ( Direction direction )
{
     switch ( type ) {
     default:
          break;
     case Type::pushable_block:
          return interactive_pushable_block.push ( direction );
     case Type::pushable_torch:
          return interactive_pushable_torch.push ( direction );
     }

     return Direction::count;
}

Void Interactive::update ( float time_delta )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::none:
          break;
     case Type::lever:
          interactive_lever.update ( time_delta );
          break;
     case Type::pushable_block:
          interactive_pushable_block.update ( time_delta );
          break;
     case Type::exit:
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

Void Lever::update ( float time_delta )
{
     cooldown_watch.tick ( time_delta );
}

Void Lever::activate ( Interactives& interactives )
{
     if ( !cooldown_watch.expired ( ) ) {
          return;
     }

     auto& interactive = interactives.get_from_tile ( activate_coordinate_x, activate_coordinate_y );
     interactive.activate ( interactives );

     // toggle
     on = !on;

     // reset the stopwatch
     cooldown_watch.reset ( c_lever_cooldown );
}

Void PushableBlock::reset ( )
{
     state = idle;
     cooldown_watch.reset ( 0.0f );
     move_direction = Direction::count;
     pushed_last_update = false;
     //moving_offset = 0;
}

Void PushableBlock::update ( float time_delta )
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
     }

     pushed_last_update = false;
}

Direction PushableBlock::push ( Direction direction )
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
               move_direction = direction;
          }
          pushed_last_update = true;
          break;
     case moving:
          state = idle;
          return direction;
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

Void PushableTorch::update ( float time_delta )
{
     pushable_block.update ( time_delta );
}

Void PushableTorch::activate ( )
{
     torch.activate ( );
}

Direction PushableTorch::push ( Direction direction )
{
     return pushable_block.push ( direction );
}


