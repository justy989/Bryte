#ifndef BRYTE_ENEMY_HPP
#define BRYTE_ENEMY_HPP

#include "Character.hpp"
#include "Pickup.hpp"
#include "Random.hpp"

namespace bryte
{
     struct Enemy : public Character {
     public:

          enum Type {
               rat,
               bat,
               goo,
               skeleton,
               fairy,
               knight,
               spike,
               ice_wizard,
               count
          };

          struct RatState {
               bool      moving; // either moving or waiting
               bool      reacting_to_attack;
               Stopwatch timer;
          };

          struct BatState {
               enum Direction {
                    up_left,
                    up_right,
                    down_left,
                    down_right,
                    count
               };

               static const Real32 c_walk_speed;
               static const Real32 c_dash_speed;

               Direction move_direction;
               Stopwatch timer;
               Bool dashing;
               Real32 target_x; // NOTE(justin): Screw you msvc not letting me have things with constructors
               Real32 target_y; //               in unions!
          };

          struct GooState {
               enum State {
                    walking,
                    picking_direction,
                    preparing_to_shoot,
                    shooting
               };

               static const Real32 c_shoot_time;

               State state;
               Stopwatch state_timer;
          };

          struct SkeletonState {
               static const Real32 c_attack_range;
               static const Real32 c_walk_speed;
               static const Real32 c_attack_speed;
               static const Real32 c_attack_delay;

               Stopwatch wander_timer;
               Stopwatch attack_timer;
          };

          struct FairyState {
               static const Real32 c_heal_radius;
               static const Real32 c_heal_delay;

               Stopwatch heal_timer;
          };

          struct KnightState {
               Stopwatch decision_timer;
               Bool reacted_to_player_attack;
          };

          struct SpikeState {
               enum Direction {
                    up_left,
                    up_right,
                    down_left,
                    down_right,
                    count
               };

               Direction move_direction;
          };

          struct IceWizardState {
               enum State {
                    idle,
                    wandering,
                    warm_up,
                    attack,
                    cool_down
               };

               static const Real32 c_warm_up_time;
               static const Real32 c_cool_down_time;

               State state;
               Stopwatch state_timer;
          };

     public:

          Void init ( Type type, Real32 x, Real32 y, Direction facing, Pickup::Type drop );

          Void think ( Enemy* enemies, Int32 max_enemies,
                       const Character& player, Random& random, float time_delta );

          Void clear ( );

     private:

          Void rat_think ( const Character& player, Random& random, float time_delta );
          Void bat_think ( const Character& player, Random& random, float time_delta );
          Void goo_think ( const Character& player, Random& random, float time_delta );
          Void skeleton_think ( const Character& player, Random& random, float time_delta );
          Void fairy_think ( Enemy* enemies, Int32 max_enemies,
                             const Character& player, Random& random, float time_delta );
          Void knight_think ( const Character& player, Random& random, float time_delta );
          Void spike_think ( const Character& player, Random& random, float time_delta );
          Void ice_wizard_think ( const Character& player, Random& random, float time_delta );
          Bool wizard_decide_whether_to_attack ( const Character& player );

     public:

          static const Char8* c_names [ Enemy::Type::count ];

     public:

          Type type;

          union {
               RatState rat_state;
               BatState bat_state;
               GooState goo_state;
               SkeletonState skeleton_state;
               FairyState fairy_state;
               KnightState knight_state;
               SpikeState spike_state;
               IceWizardState ice_wizard_state;
          };

          Pickup::Type drop;

          Bool hit_player;
     };
}

#endif

