#include "Enemy.hpp"
#include "Utils.hpp"
#include "Map.hpp"

#include <cmath>

using namespace bryte;

const Real32 Enemy::GooState::c_shoot_time           = 2.0f;
const Real32 Enemy::SkeletonState::c_attack_range    = 6.0f;
const Real32 Enemy::SkeletonState::c_walk_speed      = 4.0f;
const Real32 Enemy::SkeletonState::c_attack_speed    = 6.0f;
const Real32 Enemy::SkeletonState::c_attack_delay    = 1.0f;
const Real32 Enemy::BatState::c_walk_speed           = 5.0f;
const Real32 Enemy::BatState::c_dash_speed           = 10.0f;
const Real32 Enemy::FairyState::c_heal_radius        = 4.0f;
const Real32 Enemy::FairyState::c_heal_delay         = 1.0f;
const Real32 Enemy::IceWizardState::c_warm_up_time   = 0.75f;
const Real32 Enemy::IceWizardState::c_cool_down_time = 2.25f;

const Char8* Enemy::c_names [ Enemy::Type::count ] = {
     "RAT",
     "BAT",
     "GOO",
     "SKELETON",
     "FAIRY",
     "KNIGHT",
     "SPIKE",
     "ICEWIZARD"
};

Void Enemy::init ( Type type, Real32 x, Real32 y, Direction facing, Pickup::Type drop  )
{
     life_state   = LifeState::alive;
     state        = State::idle;

     this->type   = type;
     this->facing = facing;
     this->drop   = drop;

     position.set ( x, y );
     velocity.zero ( );
     acceleration.zero ( );

     damage_pushed = Direction::left;

     effected_by_element = Element::none;
     on_ice = false;
     on_moving_walkway = Direction::count;

     state_watch.reset ( 0.0f );
     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );
     element_watch.reset ( 0.0f );

     walk_tracker = 0.0f;
     walk_frame = 0;

     collides_with_exits  = true;

     switch ( type ) {
     default:
          break;
     case Enemy::Type::rat:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 1 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 12 ), pixels_to_meters ( 10 ) );

          flies = false;
          knockbackable = true;

          walk_acceleration = 9.0f;
          deceleration_scale = 5.0f;

          walk_frame_change = 0;
          walk_frame_count = 1;
          walk_frame_rate = 50.0f;
          constant_animation = false;
          draw_facing = true;

          rat_state.moving = false;
          rat_state.reacting_to_attack = false;
          rat_state.timer.reset ( 0.0f );
          break;
     case Enemy::Type::bat:
          health     = 1;
          max_health = 1;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 4 ) );
          collision_dimension.set ( pixels_to_meters ( 10 ), pixels_to_meters ( 6 ) );

          flies = true;
          knockbackable = true;

          walk_acceleration = BatState::c_walk_speed;
          deceleration_scale = 2.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 0.2f;
          constant_animation = true;
          draw_facing = false;

          bat_state.move_direction = BatState::Direction::up_left;
          bat_state.timer.reset ( 0.0f );
          bat_state.dashing = false;
          bat_state.target_x = 0.0f;
          bat_state.target_y = 0.0f;
          break;
     case Enemy::Type::goo:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 2 ) );
          collision_dimension.set ( pixels_to_meters ( 10 ), pixels_to_meters ( 10 ) );

          flies = false;
          knockbackable = true;

          walk_acceleration = 3.5f;
          deceleration_scale = 2.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 0.4f;
          constant_animation = true;
          draw_facing = true;

          goo_state.state = GooState::State::picking_direction;
          goo_state.state_timer.reset ( 0.0f );
          break;
     case Enemy::Type::skeleton:
          health     = 3;
          max_health = 3;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 1 ) );
          collision_dimension.set ( pixels_to_meters ( 14 ), pixels_to_meters ( 14 ) );

          flies = false;
          knockbackable = true;

          walk_acceleration = 3.0f;
          deceleration_scale = 2.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 15.0f;
          constant_animation = false;

          draw_facing = false;

          break;
     case Enemy::Type::fairy:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 4 ), pixels_to_meters ( 1 ) );
          collision_dimension.set ( pixels_to_meters ( 6 ), pixels_to_meters ( 12 ) );

          flies = true;
          knockbackable = false;

          walk_acceleration = 4.0f;
          deceleration_scale = 2.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 0.2f;
          constant_animation = true;

          draw_facing = false;

          fairy_state.heal_timer.reset ( 0.0f );
          break;
     case Enemy::Type::knight:
          health     = 5;
          max_health = 5;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 1 ), pixels_to_meters ( 1 ) );
          collision_dimension.set ( pixels_to_meters ( 15 ), pixels_to_meters ( 15 ) );

          flies = false;
          knockbackable = false;

          walk_acceleration = 6.0f;
          deceleration_scale = 3.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 15.0f;
          constant_animation = false;

          draw_facing = true;

          knight_state.decision_timer.reset ( 0.0f );
          knight_state.reacted_to_player_attack = false;
          break;
     case Enemy::Type::spike:
          health     = 2;
          max_health = 2;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 1 ), pixels_to_meters ( 1 ) );
          collision_dimension.set ( pixels_to_meters ( 15 ), pixels_to_meters ( 15 ) );

          flies = false;
          knockbackable = false;

          walk_acceleration = 6.0f;
          deceleration_scale = 3.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 0.3f;
          constant_animation = true;

          draw_facing = true;

          spike_state.move_direction = SpikeState::Direction::count;
          break;
     case Enemy::Type::ice_wizard:
          health     = 3;
          max_health = 3;

          dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
          collision_offset.set ( pixels_to_meters ( 2 ), pixels_to_meters ( 3 ) );
          collision_dimension.set ( pixels_to_meters ( 14 ), pixels_to_meters ( 12 ) );

          flies = false;
          knockbackable = false;

          walk_acceleration = 6.0f;
          deceleration_scale = 3.0f;

          walk_frame_change = 1;
          walk_frame_count = 3;
          walk_frame_rate = 15.0f;
          constant_animation = false;

          draw_facing = true;

          ice_wizard_state.state = IceWizardState::wandering;
          ice_wizard_state.state_timer.reset ( 0.0f );
          break;
     }
}

Void Enemy::think ( Enemy* enemies, Int32 max_enemies,
                    const Character& player, Random& random, float time_delta )
{
     switch ( type )
     {
     default:
          break;
     case Type::rat:
          rat_think ( player, random, time_delta );
          break;
     case Type::bat:
          bat_think ( player, random, time_delta );
          break;
     case Type::goo:
          goo_think ( player, random, time_delta );
          break;
     case Type::skeleton:
          skeleton_think ( player, random, time_delta );
          break;
     case Type::fairy:
          fairy_think ( enemies, max_enemies, player, random, time_delta );
          break;
     case Type::knight:
          knight_think ( player, random, time_delta );
          break;
     case Type::spike:
          spike_think ( player, random, time_delta );
          break;
     case Type::ice_wizard:
          ice_wizard_think ( player, random, time_delta );
          break;
     }

     hit_player = false;
}

Void Enemy::clear ( )
{
     state = State::idle;

     facing = Direction::left;
     state_watch.reset ( 0.0f );

     health = 0;
     max_health = 0;

     dimension.zero ( );
     acceleration.zero ( );

     collision_offset.zero ( );
     collision_dimension.zero ( );

     walk_acceleration = 0.0f;

     damage_pushed = Direction::count;

     damage_watch.reset ( 0.0f );
     cooldown_watch.reset ( 0.0f );

     position.zero ( );

     flies = false;
     knockbackable = true;

     type = Type::count;
     drop = Pickup::Type::none;
}

Void Enemy::rat_think ( const Character& player, Random& random, float time_delta )
{
     Bool& moving             = rat_state.moving;
     Bool& reacting_to_attack = rat_state.reacting_to_attack;
     Stopwatch& timer         = rat_state.timer;

     // if attacked move in a
     if ( state == Character::State::blinking && !reacting_to_attack ) {
          change_facing ( static_cast<Direction>( random.generate ( 0, Direction::count ) ) );
          timer.reset ( static_cast<Real32>( random.generate ( 1, 3 ) ) );
          moving             = true;
          reacting_to_attack = true;
     }

     if ( !moving ) {
          timer.tick ( time_delta );

          if ( timer.expired ( ) ) {
               // choose a new direction, and start moving
               change_facing ( static_cast<Direction>( random.generate ( 0, Direction::count ) ) );
               timer.reset ( static_cast<Real32>( random.generate ( 0, 4 ) ) );
               moving = true;
          }
     } else {
          if ( collided_last_frame != Direction::count ) {
               moving = false;
               timer.reset ( 0.0f );
          } else {
               walk ( facing );

               timer.tick ( time_delta );

               if ( timer.expired ( ) ) {
                    timer.reset ( static_cast<Real32>( random.generate ( 0, 3 ) ) );
                    moving             = false;
                    reacting_to_attack = false;
               }
          }
     }
}

Void Enemy::bat_think ( const Character& player, Random& random, float time_delta )
{
     Stopwatch& timer     = bat_state.timer;
     Auto& move_direction = bat_state.move_direction;
     Bool& dashing        = bat_state.dashing;

     timer.tick ( time_delta );

     if ( dashing ) {
          if ( collided_last_frame != Direction::count ) {
               // TODO: pathfinding so we can always have the bat find you?
               dashing = false;
               walk_acceleration = BatState::c_walk_speed;
          } else {
               Vector target { bat_state.target_x, bat_state.target_y };
               Real32 distance_to_target = position.distance_to ( target );
               Direction dir = direction_between ( position, target, random );
               walk ( dir );
               walk_acceleration = BatState::c_dash_speed;

               if ( distance_to_target < Map::c_tile_dimension_in_meters ) {
                    dashing = false;
                    walk_acceleration = BatState::c_walk_speed;
               }
               return;
          }
     }

     if ( timer.expired ( ) || collided_last_frame != Direction::count ) {
          move_direction = static_cast<BatState::Direction>( random.generate ( 0, BatState::Direction::count + 1 ) );
          timer.reset ( static_cast<Real32>( random.generate ( 1, 2 ) ) );
     }

     switch ( move_direction ) {
     default:
          break;
     case BatState::Direction::up_left:
          walk ( Direction::up );
          walk ( Direction::left );
          break;
     case BatState::Direction::up_right:
          walk ( Direction::up );
          walk ( Direction::right );
          break;
     case BatState::Direction::down_left:
          walk ( Direction::down );
          walk ( Direction::left );
          break;
     case BatState::Direction::down_right:
          walk ( Direction::down );
          walk ( Direction::right );
          break;
     }

     if ( player.is_attacking ( ) && !dashing ) {
          dashing = true;
          Vector player_center = player.collision_center ( );
          bat_state.target_x = player_center.x ( );
          bat_state.target_y = player_center.y ( );
     }
}

Void Enemy::goo_think ( const Character& player, Random& random, float time_delta )
{
     GooState::State& state = goo_state.state;
     Stopwatch& state_timer = goo_state.state_timer;

     state_timer.tick ( time_delta );

     switch ( state ) {
     default:
          break;
     case GooState::State::walking:
          if ( collided_last_frame != Direction::count ) {
               state = GooState::State::picking_direction;
          } else {
               walk ( facing );

               if ( state_timer.expired ( ) ) {
                    state = GooState::State::picking_direction;
               }
          }
          break;
     case GooState::State::picking_direction:
          change_facing ( static_cast<Direction>( random.generate ( 0, Direction::count ) ) );
          state_timer.reset ( 1 );
          state= GooState::State::preparing_to_shoot;
          break;
     case GooState::State::preparing_to_shoot:
          if ( state_timer.expired ( ) && effected_by_element != Element::ice ) {
               state = GooState::State::shooting;
          }
          break;
     case GooState::State::shooting:
          state_timer.reset ( static_cast<Real32>( random.generate ( 0, 3 ) ) );
          state = GooState::State::walking;
          break;
     }
}

Void Enemy::skeleton_think ( const Character& player, Random& random, float time_delta )
{
     Auto& wander_timer = skeleton_state.wander_timer;
     Auto& attack_timer = skeleton_state.attack_timer;
     Real32 distance_to_player = position.distance_to ( player.position );

     if ( !attack_timer.expired ( ) ) {
          attack_timer.tick ( time_delta );
          return;
     }

     // if we have taken damage, pause until we have stopped blinking
     if ( !damage_watch.expired ( ) || hit_player ) {
          attack_timer.reset ( SkeletonState::c_attack_delay );
          return;
     }

     if ( distance_to_player < SkeletonState::c_attack_range ) {
          walk ( direction_between ( position, player.position, random ) );
          walk_acceleration = SkeletonState::c_attack_speed;
     } else {
          wander_timer.tick ( time_delta );

          walk ( facing );

          if ( wander_timer.expired ( ) || collided_last_frame != Direction::count ) {
               change_facing ( static_cast<Direction>( random.generate ( 0, Direction::count ) ) );
               wander_timer.reset ( static_cast<Real32>( random.generate ( 0, 3 ) ) );
          }

          walk_acceleration = SkeletonState::c_walk_speed;
     }
}

Void Enemy::fairy_think ( Enemy* enemies, Int32 max_enemies,
                          const Character& player, Random& random, float time_delta )
{
     Vector average;
     Real32 enemy_count = 0.0f;

     for ( Int32 i = 0; i < max_enemies; ++i ) {
          if ( enemies [ i ].is_dead ( ) ||
               enemies [ i ].type == Enemy::Type::fairy ) {
               continue;
          }

          Vector enemy_pos = enemies [ i ].position;

          average += enemy_pos;
          enemy_count += 1.0f;
     }

     average /= enemy_count;

     if ( enemy_count > 0.0f ) {
          Direction dir = direction_between ( position, average, random );
          walk ( dir );
     }

     if ( fairy_state.heal_timer.expired ( ) ) {
          fairy_state.heal_timer.reset ( FairyState::c_heal_delay );
     }

     fairy_state.heal_timer.tick ( time_delta );
}

Void Enemy::knight_think ( const Character& player, Random& random, float time_delta )
{
     Auto& decision_timer = knight_state.decision_timer;
     Auto& reacted_to_player_attack = knight_state.reacted_to_player_attack;

     decision_timer.tick ( time_delta );

     if ( collided_last_frame != Direction::count || decision_timer.expired ( ) ) {
          decision_timer.reset ( static_cast<Real32>( random.generate ( 0, 3 ) ) );

          // Turn or keep moving forward?
          Auto decision = random.generate ( 0, 2 );

          if ( decision == 1 ) {
               decision = random.generate ( 0, 2 );

               if ( decision == 0 ) {
                    walk ( left_turn ( facing ) );
               } else if ( decision == 1 ) {
                    walk ( right_turn ( facing ) );
               }
          }

          // reset ability to react to the player attacking
          reacted_to_player_attack = false;
          return;
     } else if ( player.is_attacking ( ) && !reacted_to_player_attack ) {
          // randomly hear player and change movement towards them
          Auto decision = random.generate ( 0, 2 );

          if ( decision == 1 ) {
               walk ( direction_between ( position, player.position, random ) );
          }

          decision_timer.reset ( static_cast<Real32>( random.generate ( 0, 3 ) ) );
          reacted_to_player_attack = true;
     }

     walk ( facing );
}

Void Enemy::spike_think ( const Character& player, Random& random, float time_delta )
{
     Auto& move_direction = spike_state.move_direction;

     if ( move_direction == SpikeState::Direction::count ) {
          move_direction = static_cast<SpikeState::Direction>( random.generate ( 0, 5 ) );
     }

     // handle collisions with environment
     switch ( move_direction ) {
     default:
          break;
     case SpikeState::Direction::up_left:
          if ( collided_last_frame == Direction::left ) {
               move_direction = SpikeState::Direction::up_right;
          } else if ( collided_last_frame == Direction::up ) {
               move_direction = SpikeState::Direction::down_left;
          }
          break;
     case SpikeState::Direction::up_right:
          if ( collided_last_frame == Direction::right ) {
               move_direction = SpikeState::Direction::up_left;
          } else if ( collided_last_frame == Direction::up ) {
               move_direction = SpikeState::Direction::down_left;
          }
          break;
     case SpikeState::Direction::down_left:
          if ( collided_last_frame == Direction::left ) {
               move_direction = SpikeState::Direction::down_right;
          } else if ( collided_last_frame == Direction::down ) {
               move_direction = SpikeState::Direction::up_left;
          }
          break;
     case SpikeState::Direction::down_right:
          if ( collided_last_frame == Direction::right ) {
               move_direction = SpikeState::Direction::down_left;
          } else if ( collided_last_frame == Direction::down ) {
               move_direction = SpikeState::Direction::up_right;
          }
          break;
     }

     // move in the resulting direction
     switch ( move_direction ) {
     default:
          break;
     case SpikeState::Direction::up_left:
          walk ( Direction::up );
          walk ( Direction::left );
          break;
     case SpikeState::Direction::up_right:
          walk ( Direction::up );
          walk ( Direction::right );
          break;
     case SpikeState::Direction::down_left:
          walk ( Direction::down );
          walk ( Direction::left );
          break;
     case SpikeState::Direction::down_right:
          walk ( Direction::down );
          walk ( Direction::right );
          break;
     }
}

Void Enemy::wizard_decide_whether_to_attack ( const Character& player )
{
     static const Real32 c_half_tile = Map::c_tile_dimension_in_meters * 0.5f;

     Auto& state = ice_wizard_state.state;
     Auto& state_timer = ice_wizard_state.state_timer;

     Vector new_player_diff = collision_center ( ) - player.collision_center ( );

     // check for player passing along each axis
     if ( fabs ( new_player_diff.x ( ) ) < c_half_tile ) {
          if ( player.effected_by_element != Element::ice ) {
               state = IceWizardState::warm_up;
               state_timer.reset ( IceWizardState::c_warm_up_time );
          }
     } else if ( fabs ( new_player_diff.y ( ) ) < c_half_tile ) {
          if ( player.effected_by_element != Element::ice ) {
               state = IceWizardState::warm_up;
               state_timer.reset ( IceWizardState::c_warm_up_time );
          }
     }
}

Void Enemy::ice_wizard_think ( const Character& player, Random& random, float time_delta )
{
     Auto& state = ice_wizard_state.state;
     Auto& state_timer = ice_wizard_state.state_timer;

     switch ( state ) {
     default:
          break;
     case IceWizardState::idle:
          state_timer.tick ( time_delta );

          if ( state_timer.expired ( ) ) {
               state = IceWizardState::wandering;
               facing = static_cast<Direction>( random.generate ( 0, BatState::Direction::count + 1 ) );
               state_timer.reset ( static_cast<Real32>( random.generate ( 1, 2 ) ) );
          }

          wizard_decide_whether_to_attack ( player );
          break;
     case IceWizardState::wandering:
          state_timer.tick ( time_delta );
          if ( state_timer.expired ( ) ) {
               state = IceWizardState::idle;
               state_timer.reset ( static_cast<Real32>( random.generate ( 1, 4 ) ) );
          } else {
               walk ( facing );
          }
          wizard_decide_whether_to_attack ( player );
          break;
     case IceWizardState::warm_up:
          state_timer.tick ( time_delta );

          if ( state_timer.expired ( ) ) {
               state = IceWizardState::attack;
          }
          break;
     case IceWizardState::attack:
          state = IceWizardState::cool_down;
          state_timer.reset ( IceWizardState::c_cool_down_time );
          break;
     case IceWizardState::cool_down:
          state_timer.tick ( time_delta );

          if ( state_timer.expired ( ) ) {
               state = IceWizardState::wandering;
          }
          break;
     }
}

