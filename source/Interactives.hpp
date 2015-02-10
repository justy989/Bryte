#ifndef BRYTE_INTERACTIVES_HPP
#define BRYTE_INTERACTIVES_HPP

#include "Types.hpp"
#include "Map.hpp"
#include "StopWatch.hpp"
#include "Direction.hpp"

namespace bryte
{
     class Interactives;

     struct Exit {
          enum State {
               open,
               closed,
               locked,
               count
          };

          Void reset ( );

          Void activate ( );

          Direction direction;
          State     state;
          Uint8     map_index;
          Uint8     exit_index_x;
          Uint8     exit_index_y;
     };

     struct Lever {
          enum State {
               off,
               on,
               changing_on,
               changing_off,
          };

          Void reset ( );

          Void update ( Real32 time_delta, Interactives& interactives );

          Void activate ( );

          State     state;
          Stopwatch cooldown_watch;
          Uint8     activate_coordinate_x;
          Uint8     activate_coordinate_y;
     };

     struct PushableBlock {
          enum State {
               idle,      // no luv at the moment
               leaned_on, // a character is pushing it
               moving,    // a single frame to tell the map I moved
               solid      // no longer pushable
          };

          Void reset ( );

          Void update ( Real32 time_delta );

          Direction push ( Direction direction, Interactives& interactives );

          State     state;
          Stopwatch cooldown_watch;
          Direction restricted_direction;
          Bool      one_time;
          Bool      pushed_last_update;
          Uint8     activate_coordinate_x;
          Uint8     activate_coordinate_y;
     };

     struct Torch {

          Void reset ( );
          Void activate ( );

          Bool  on;
          Int32 value;
     };

     struct PushableTorch {

          Void reset ( );

          Void update ( Real32 time_delta );

          Void activate  ( );
          Direction push ( Direction direction, Interactives& interactives );

          Torch torch;
          PushableBlock pushable_block;
     };

     struct LightDetector {

          Void reset ( );

          Void light ( Uint8 value, Interactives& interactives );

          enum Type {
               bryte,
               dark,
          };

          Type  type;
          bool  below_value;
          Uint8 activate_coordinate_x;
          Uint8 activate_coordinate_y;

          static const Uint8 c_bryte_value = 178;
          static const Uint8 c_dark_value  = 128;
     };

     struct PressurePlate {
          Bool  entered;
          Uint8 activate_coordinate_x;
          Uint8 activate_coordinate_y;
     };

     struct PopupBlock {
          Bool up;
     };

     struct UnderneathInteractive {
          enum Type {
               none,
               pressure_plate,
               popup_block
          };

          Void reset ( );

          Type type;

          union {
               PressurePlate underneath_pressure_plate;
               PopupBlock    underneath_popup_block;
          };
     };

     struct Interactive {
          enum Type {
               none,
               lever,
               pushable_block,
               torch,
               pushable_torch,
               light_detector,
               exit,
               count
          };

          Void      reset    ( );

          Void      update   ( Real32 time_delta, Interactives& interactives );

          Void      activate ( Interactives& interactives );
          Direction push     ( Direction direction, Interactives& interactives );
          Void      light    ( Uint8 light, Interactives& interactives );
          Void      enter    ( Interactives& interactives );
          Void      leave    ( Interactives& interactives );

          Bool is_solid ( ) const;

          Type type;

          union {
               Lever         interactive_lever;
               PushableBlock interactive_pushable_block;
               Exit          interactive_exit;
               Torch         interactive_torch;
               PushableTorch interactive_pushable_torch;
               LightDetector interactive_light_detector;
          };

          UnderneathInteractive underneath;
     };

     class Interactives {
     public:

          Void reset ( Int32 width, Int32 height );

          Interactive& add ( Interactive::Type type, Int32 tile_x, Int32 tile_y );

          Void update ( Real32 time_delta );

          Void contribute_light ( Map& map );

          Void push ( Int32 tile_x, Int32 tile_y, Direction dir, const Map& map );
          Void activate ( Int32 tile_x, Int32 tile_y );
          Void light ( Int32 tile_x, Int32 tile_y, Uint8 light );
          Void enter ( Int32 tile_x, Int32 tile_y );
          Void leave ( Int32 tile_x, Int32 tile_y );

          Interactive& get_from_tile ( Int32 tile_x, Int32 tile_y );
          const Interactive& cget_from_tile ( Int32 tile_x, Int32 tile_y ) const;

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
          return m_width;
     }

     inline Int32 Interactives::height ( ) const
     {
          return m_height;
     }
}

#endif

