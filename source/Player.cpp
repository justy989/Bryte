#include "Player.hpp"
#include "Utils.hpp"
#include "Log.hpp"

#include <fstream>

using namespace bryte;

const Real32 Player::c_item_cooldown = 1.0f;

Void Player::clear ( )
{
     state = State::idle;

     facing = Direction::up;
     state_watch.reset ( 0.0f );

     health           = 6;
     max_health       = 6;

     velocity.zero ( );
     acceleration.zero ( );

     dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );

     collision_offset.set ( pixels_to_meters ( 5 ), pixels_to_meters ( 2 ) );
     collision_dimension.set ( pixels_to_meters ( 6 ), pixels_to_meters ( 7 ) );

     walk_acceleration = 18.0f;
     deceleration_scale = 6.0f;

     walk_tracker = 0.0f;
     walk_frame = 0;
     walk_frame_count = 3;
     walk_frame_rate = 30.0f;
     walk_frame_change = 1;
     constant_animation = false;
     draw_facing = true;

     damage_pushed = Direction::count;

     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );
     element_watch.reset ( 0.0f );

     effected_by_element = Element::none;
     on_ice = false;
     on_moving_walkway = Direction::count;

     position.zero ( );

     flies = false;
     knockbackable = true;
     collides_with_exits = false;

     item_mode = ItemMode::shield;
     item_cooldown.reset ( 0.0f );

     key_count   = 0;
     arrow_count = 0;
     bomb_count  = 0;

     save_slot = 0;
}

Bool Player::save ( )
{
     char filepath [ 128 ];

     sprintf ( filepath, "save/slot_%d.brs", save_slot );

     std::ofstream file ( filepath, std::ios::binary );

     LOG_INFO ( "Saving player info: %s\n", filepath );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open file.\n" );
          return false;
     }

     file.write ( reinterpret_cast<const Char8*>( &health ), sizeof ( health ) );
     file.write ( reinterpret_cast<const Char8*>( &max_health ), sizeof ( max_health ) );

     file.write ( reinterpret_cast<const Char8*>( &key_count ), sizeof ( key_count ) );
     file.write ( reinterpret_cast<const Char8*>( &arrow_count ), sizeof ( arrow_count ) );
     file.write ( reinterpret_cast<const Char8*>( &bomb_count ), sizeof ( bomb_count ) );

     file.close ( );

     return true;
}

Bool Player::load ( )
{
     char filepath [ 128 ];

     sprintf ( filepath, "save/slot_%d.brs", save_slot );

     LOG_INFO ( "Loading player info: %s\n", filepath );

     std::ifstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open file.\n" );
          return false;
     }

     file.read ( reinterpret_cast<Char8*>( &health ), sizeof ( health ) );
     file.read ( reinterpret_cast<Char8*>( &max_health ), sizeof ( max_health ) );

     file.read ( reinterpret_cast<Char8*>( &key_count ), sizeof ( key_count ) );
     file.read ( reinterpret_cast<Char8*>( &arrow_count ), sizeof ( arrow_count ) );
     file.read ( reinterpret_cast<Char8*>( &bomb_count ), sizeof ( bomb_count ) );

     file.close ( );

     return true;
}

