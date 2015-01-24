#ifndef BRYTE_INTERACTIVES_HPP
#define BRYTE_INTERACTIVES_HPP

#include "Types.hpp"
#include "Map.hpp"
#include "StopWatch.hpp"
#include "Direction.hpp"

namespace bryte
{
     class Interactives;

     struct Lever {
          Void reset ( );

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

          Void reset ( );

          Void update ( float time_delta );

          Direction push ( Direction direction );

          State     state;
          Stopwatch cooldown_watch;
          Direction move_direction;
          Bool      pushed_last_update;
          Int32     moving_offset;
     };

     struct Exit {
          enum State {
               open,
               closed,
               locked,
               count
          };

          Void activate ( );

          Direction direction;
          State     state;
          Uint8     map_index;
          Uint8     exit_index_x;
          Uint8     exit_index_y;
     };

     struct Interactive {
          enum Type {
               none,
               lever,
               pushable_block,
               exit,
               count
          };

          Void      reset    ( );

          Void      update   ( float time_delta );

          Void      activate ( Map& map );
          Direction push     ( Direction direction );

          Bool is_solid ( ) const;

          Type type;

          union {
               Lever         interactive_lever;
               PushableBlock interactive_pushable_block;
               Exit          interactive_exit;
          };
     };

     class Interactives {
     public:

          Void reset ( Int32 width, Int32 height );

          Interactive& add ( Interactive::Type type, Int32 tile_x, Int32 tile_y );

          Void update ( float time_delta );

          Void push ( Int32 tile_x, Int32 tile_y, Direction dir, const Map& map );
          Void activate ( Int32 tile_x, Int32 tile_y, Map& map );

          Interactive& get_from_tile ( Int32 tile_x, Int32 tile_y );

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

