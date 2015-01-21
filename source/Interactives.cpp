#include "Interactives.hpp"
#include "Log.hpp"

using namespace bryte;

static const Real32 c_lever_cooldown     = 0.75f;
static const Real32 c_lean_on_block_time = 0.6f;

const Interactive& Interactives::interactive ( Int32 tile_x, Int32 tile_y ) const
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     return m_interactives [ tile_y * m_width + tile_x ];
}

Interactive& Interactives::get_interactive ( Int32 tile_x, Int32 tile_y )
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     return m_interactives [ tile_y * m_width + tile_x ];
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
     auto& i = get_interactive ( tile_x, tile_y );

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
     Interactive& i = get_interactive ( tile_x, tile_y );

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

     Interactive& dest_i = get_interactive ( dest_x, dest_y );

     if ( dest_i.type == Interactive::Type::none &&
          !map.get_coordinate_solid ( dest_x, dest_y ) ) {
          dest_i = i;
          i.type = Interactive::Type::none;
     }
}

Void Interactives::activate ( Int32 tile_x, Int32 tile_y, Map& map )
{
     Interactive& i = get_interactive ( tile_x, tile_y );

     i.activate ( map );
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
     }

     return false;
}

Void Interactive::reset ( )
{
     switch ( type ) {
     default:
          break;
     case Type::lever:
          interactive_lever.reset ( );
          break;
     case Type::pushable_block:
          interactive_pushable_block.reset ( );
          break;
     }
}

Void Interactive::activate ( Map& map )
{
     switch ( type ) {
     default:
          break;
     case Type::lever:
          interactive_lever.activate ( map );
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
     }

     return Direction::count;
}

Void Interactive::update ( float time_delta )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::lever:
          interactive_lever.update ( time_delta );
          break;
     case Type::pushable_block:
          interactive_pushable_block.update ( time_delta );
          break;
     case Type::exit:
          break;
     }
}

Void Lever::reset ( )
{
     cooldown_watch.reset ( 0.0f );
     on                       = false;
     change_tile_coordinate_x = 0;
     change_tile_coordinate_y = 0;
     change_tile_value        = 0;
}

Void Lever::update ( float time_delta )
{
     cooldown_watch.tick ( time_delta );
}

Void Lever::activate ( Map& map )
{
     if ( !cooldown_watch.expired ( ) ) {
          return;
     }

     auto tile_value = map.get_coordinate_value ( change_tile_coordinate_x,
                                                  change_tile_coordinate_y );
     auto tile_solid = map.get_coordinate_solid ( change_tile_coordinate_x,
                                                  change_tile_coordinate_y );

     // toggle
     on = !on;

     map.set_coordinate_value ( change_tile_coordinate_x, change_tile_coordinate_y,
                                change_tile_value );

     map.set_coordinate_solid ( change_tile_coordinate_x, change_tile_coordinate_y,
                                !tile_solid );

     change_tile_value = tile_value;

     // reset the stopwatch
     cooldown_watch.reset ( c_lever_cooldown );
}

Void PushableBlock::reset ( )
{
     state = idle;
     cooldown_watch.reset ( 0.0f );
     move_direction = Direction::count;
     pushed_last_update = false;
     moving_offset = 0;
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

Void Exit::activate ( )
{
     if ( state == State::closed || state == State::locked ) {
          state = State::open;
     }
}

