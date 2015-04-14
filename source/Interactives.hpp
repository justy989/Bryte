#ifndef BRYTE_INTERACTIVES_HPP
#define BRYTE_INTERACTIVES_HPP

#include "Types.hpp"
#include "Map.hpp"
#include "StopWatch.hpp"
#include "Direction.hpp"
#include "Element.hpp"

namespace bryte
{
     struct Interactives;
     struct Character;
     struct Projectile;

     struct Exit {
          enum State {
               closed,
               changing_to_open,
               open,
               changing_to_unlocked,
               locked,
               changing_to_closed,
               changing_to_locked,
               count
          };

          Bool changing ( ) const;

          Void reset ( );

          Void update ( Real32 time_delta );

          Bool activate ( );

          Direction direction;
          Stopwatch state_watch;
          State     state;
          Uint8     map_index;
          Uint8     exit_index_x;
          Uint8     exit_index_y;
          Uint8     region_index;
     };

     struct Lever {
          enum State {
               off,
               on,
               changing_on,
               changing_off,
          };

          Bool changing ( ) const;

          Void reset ( );

          Void update ( Real32 time_delta, Interactives& interactives );

          Bool activate ( );

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
          Bool      one_time;
          Bool      pushed_last_update;
          Uint8     activate_coordinate_x;
          Uint8     activate_coordinate_y;
     };

     struct Torch {
          Void reset ( );
          Bool activate ( );

          Element element;
          Int32 value;
     };

     struct PushableTorch {

          Void reset ( );

          Void update ( Real32 time_delta );

          Bool activate  ( );
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

     struct Turret {

          Void reset ( );

          Bool activate ( );

          Void update ( Real32 time_delta );

          static const Real32 c_shoot_interval;

          bool automatic;
          bool wants_to_shoot;
          Stopwatch automatic_watch;
          Direction facing;
     };

     struct IceDetector {
          Void reset ( );

          Bool detected;
          Direction force_dir;
          Uint8 activate_coordinate_x;
          Uint8 activate_coordinate_y;
     };

     struct Portal {
          Void reset ( );
          Bool activate ( );
          Direction push ( Direction direction, Interactives& interactives );

          Uint8 destination_x;
          Uint8 destination_y;
     };

     struct PressurePlate {
          Bool  entered;
          Uint8 activate_coordinate_x;
          Uint8 activate_coordinate_y;
     };

     struct PopupBlock {
          Bool up;
     };

     struct Ice {
          Void reset ( );

          Direction force_dir;
     };

     struct MovingWalkway {
          Void reset ( );

          Direction facing;
     };

     struct Hole {
          Void reset ( );

          Bool filled;
     };

     struct Destructable {
          Void reset ( );

          Bool destroyed;
     };

     struct UnderneathInteractive {
          enum Type {
               none,
               pressure_plate,
               popup_block,
               ice,
               moving_walkway,
               light_detector,
               ice_detector,
               hole,
               destructable,
               count
          };

          Void reset ( );

          Type type;

          union {
               PressurePlate underneath_pressure_plate;
               PopupBlock    underneath_popup_block;
               Ice           underneath_ice;
               MovingWalkway underneath_moving_walkway;
               LightDetector underneath_light_detector;
               IceDetector   underneath_ice_detector;
               Hole          underneath_hole;
               Destructable  underneath_destructable;
          };
     };

     struct Interactive {
          enum Type {
               none,
               lever,
               pushable_block,
               torch,
               pushable_torch,
               exit,
               bombable_block,
               turret,
               portal,
               count
          };

          Void      reset    ( );

          Void      update   ( Real32 time_delta, Interactives& interactives );

          Bool      activate ( Interactives& interactives );
          Void      explode  ( Interactives& interactives );
          Direction push     ( Direction direction, Interactives& interactives );
          Void      light    ( Uint8 light, Interactives& interactives );
          Void      attack   ( Interactives& interactives );
          Void      character_enter   ( Direction from, Interactives& interactives, Character& character );
          Void      character_leave   ( Direction to, Interactives& interactives, Character& character );
          Void      interactive_enter ( Direction from, Interactives& interactives );
          Void      interactive_leave ( Direction to, Interactives& interactives );
          Void      projectile_enter  ( Direction from, Interactives& interactives, Projectile& projectile );

          Interactive::Type type;

          union {
               Lever         interactive_lever;
               PushableBlock interactive_pushable_block;
               Exit          interactive_exit;
               Torch         interactive_torch;
               PushableTorch interactive_pushable_torch;
               Turret        interactive_turret;
               Portal        interactive_portal;
          };

          UnderneathInteractive underneath;
     };

     struct Interactives {
     public:

          Void reset ( Int32 width, Int32 height );

          Interactive& add ( Interactive::Type type, const Location& tile );

          Void contribute_light ( Map& map );

          Bool push ( const Location& tile, Direction dir, const Map& map );
          Bool activate ( const Location& tile );
          Void explode ( const Location& tile );
          Void light ( const Location& tile, Uint8 light );
          Void attack ( const Location& tile );
          Void character_enter ( const Location& tile, Character& character );
          Void character_leave ( const Location& tile, Character& character );
          Void projectile_enter ( const Location& tile, Projectile& projectile );

          Bool is_walkable ( const Location& tile, Direction dir ) const;
          Bool is_flyable ( const Location& tile ) const;

          Void spread_ice ( const Location& tile, const Map& map, bool clear = false );

          Void get_portal_destination ( Location* dest_tile, Direction dir ) const;

          Interactive& get_from_tile ( const Location& tile );
          const Interactive& cget_from_tile ( const Location& tile ) const;

          inline Int32 width ( ) const;
          inline Int32 height ( ) const;

     private:

          Void get_portal_destination_impl ( const Location& start_tile,
                                             Location* dest_tile,
                                             Direction dir ) const;

     public:

          static const Int32 c_max_interactives = Map::c_max_tiles;

     public:

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

