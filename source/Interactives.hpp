#ifndef BRYTE_INTERACTIVES_HPP
#define BRYTE_INTERACTIVES_HPP

#include "Types.hpp"
#include "Map.hpp"
#include "StopWatch.hpp"
#include "Direction.hpp"

namespace bryte
{
     struct Lever {
          Void update ( float time_delta );

          Void activate ( Map& map );

          Stopwatch cooldown_watch;
          Bool      on;
          Int32     change_tile_coordinate_x;
          Int32     change_tile_coordinate_y;
          Int32     change_tile_value;
     };

     struct PushableBlock {
          enum State {
               idle,
               leaned_on,
               moving
          };

          Void update ( float time_delta );

          Void push ( Direction direction );

          State     state;
          Stopwatch cooldown_watch;
          Direction move_direction;
          Bool      pushed_last_update;
          Int32     moving_offset;
     };

     struct Interactive {
          enum Type {
               none,
               lever,
               pushable_block,
          };

          Void update   ( float time_delta );

          Void activate ( Map& map );
          Void push     ( Direction direction );

          Bool is_solid ( ) const;

          Type type;

          union {
               Lever         interactive_lever;
               PushableBlock interactive_pushable_block;
          };
     };

     class Interactives {
     public:

          Void reset ( Int32 width, Int32 height );

          Interactive& interactive ( Int32 tile_x, Int32 tile_y );

          inline Int32 width ( ) const;
          inline Int32 height ( ) const;

     public:

          static const Int32 c_max_interactives = Map::c_max_tiles;

     private:

          Interactive m_interactives [ c_max_interactives ];

          Int32 m_width;
          Int32 m_height;
     };

     inline Int32 Interactives::width ( ) const
     {
          return m_height;
     }

     inline Int32 Interactives::height ( ) const
     {
          return m_height;
     }
}

#endif

