#include "Player.hpp"
#include "Utils.hpp"

using namespace bryte;

Void Player::clear ( )
{
     state = State::idle;

     facing = Direction::left;
     state_watch.reset ( 0.0f );

     health           = 25;
     max_health       = 25;

     velocity.zero ( );
     acceleration.zero ( );

     dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );

     collision_offset.set ( pixels_to_meters ( 5 ), pixels_to_meters ( 2 ) );
     collision_dimension.set ( pixels_to_meters ( 6 ), pixels_to_meters ( 7 ) );

     walk_acceleration.set ( 8.5f, 8.5f );

     damage_pushed = Direction::count;

     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );

     position.zero ( );

     collides_with_solids = true;
     collides_with_exits = false;

     attack_mode = AttackMode::sword;
     key_count   = 0;
}

