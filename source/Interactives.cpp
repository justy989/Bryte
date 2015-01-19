#include "Interactives.hpp"

using namespace bryte;

static const Real32 c_lever_cooldown     = 0.75f;
static const Real32 c_lean_on_block_time = 2.0f;

Interactive& Interactives::interactive ( Int32 tile_x, Int32 tile_y )
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     return m_interactives [ tile_y * m_width + tile_x ];
}

Void Interactives::reset ( Int32 width, Int32 height )
{
     m_width  = width;
     m_height = height;

     for ( int y = 0; y < height; ++y ) {
          for ( int x = 0; x < width; ++x ) {
               m_interactives [ y * width + x ].type = Interactive::Type::none;
          }
     }
}

Bool Interactive::is_solid ( ) const
{
     switch ( type ) {
     default:
          break;
     case pushable_block:
          return true;
     }

     return false;
}

Void Interactive::activate ( Map& map )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::lever:
          interactive_lever.activate ( map );
          break;
     }
}

Void Interactive::push ( Direction direction )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::pushable_block:
          interactive_pushable_block.push ( direction );
          break;
     }
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
     }
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

     map.set_coordinate_value ( change_tile_coordinate_x,
                                change_tile_coordinate_y,
                                change_tile_value );

     map.set_coordinate_solid ( change_tile_coordinate_x,
                                change_tile_coordinate_y,
                                !tile_solid );

     change_tile_value = tile_value;

     // reset the stopwatch
     cooldown_watch.reset ( c_lever_cooldown );
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
          state = idle;
          break;
     }

     pushed_last_update = false;
}

Void PushableBlock::push ( Direction direction )
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
          break;
     }
}

