#include "Player.hpp"
#include "Utils.hpp"

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

     collides_with_solids = true;
     collides_with_exits = false;

     item_mode = ItemMode::arrow;
     item_cooldown.reset ( 0.0f );
     key_count   = 0;
     arrow_count = 0;
     bomb_count  = 0;
}

