#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Sound.hpp"

#include "Region.hpp"
#include "Map.hpp"
#include "Interactives.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Pickup.hpp"
#include "Projectile.hpp"

#include "EntityManager.hpp"

#include "Random.hpp"

#include "Vector.hpp"
#include "Rect.hpp"

#include "MapDisplay.hpp"
#include "CharacterDisplay.hpp"
#include "InteractivesDisplay.hpp"
#include "PickupDisplay.hpp"
#include "ProjectileDisplay.hpp"

#include "Emitter.hpp"

#include "Text.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     struct Bomb : public Entity {
     public:

          Void update ( float dt );
          Void clear ( );

     public:

          static const Real32 c_explode_time;
          static const Real32 c_explode_radius;

     public:

          Stopwatch explode_watch;
     };

     struct DamageNumber : public Entity {
     public:

          Void update ( float time_delta );
          Void clear ( );

     public:

          static const Real32 c_rise_height;
          static const Real32 c_rise_speed;

     public:

          Stopwatch life_watch;
          Int32 value;
          Real32 starting_y;
     };

     struct Settings {
          const Char8* map_master_list_filename;

          Uint32       map_index;

          Int32        player_spawn_tile_x;
          Int32        player_spawn_tile_y;
     };

     struct State {
     public:

          Bool initialize ( GameMemory& game_memory, Settings* settings );
          Void destroy    ( );

          Bool spawn_enemy ( const Vector& position, Uint8 id, Direction facing, Pickup::Type drop );
          Bool spawn_pickup ( const Vector& position, Pickup::Type type );
          Bool spawn_projectile ( Projectile::Type type, const Vector& position, Direction facing,
                                  Projectile::Alliance alliance );
          Bool spawn_bomb ( const Vector& position );
          Bool spawn_damage_number ( const Vector& position, Int32 value );
          Bool spawn_healing_number ( const Vector& position, Int32 value );

          Void persist_map ( );
          Void spawn_map_enemies ( );

          Void tick_character_element ( Character& character );
          Void damage_character ( Character& character, Int32 amount, Direction direction );

          Void update_player ( float time_delta );
          Void player_death ( );
          Bool check_player_block_projectile ( Projectile& projectile );

          Void update_enemies ( float time_delta );
          Void enemy_death ( const Enemy& enemy );

          Void update_interactives ( float time_delta );

          Void update_projectiles ( float time_delta );
          Void update_bombs ( float time_delta );
          Void update_pickups ( float time_delta );
          Void update_emitters ( float time_delta );
          Void update_damage_numbers ( float time_delta );
          Void update_light ( );

          Void setup_emitters_from_map_lamps ( );

          Void enqueue_pickup ( Pickup::Type type );
          Void dequeue_pickup ( );

          Void heal_enemies_in_range_of_fairy ( const Vector& position );

     public:

          static const Int32 c_bomb_damage = 4;
          static const Int32 c_attack_damage = 1;
          static const Int32 c_block_damage = 0;
          static const Int32 c_enemy_damage = 1;
          static const Int32 c_burn_damage = 1;

          static const Int32 c_pickup_queue_size = 8;
          static const Real32 c_pickup_show_time;

     public:

          Sound sound;

          Random random;

          Player player;

          EntityManager<Enemy,        32> enemies;
          EntityManager<Pickup,        8> pickups;
          EntityManager<Projectile,   64> projectiles;
          EntityManager<Bomb,          8> bombs;
          EntityManager<Emitter,      32> emitters;
          EntityManager<DamageNumber, 16> damage_numbers;

          Region       region;
          Map          map;
          Interactives interactives;

          Vector       camera;

          Text text;

          SDL_PixelFormat      back_buffer_format;

          MapDisplay          map_display;
          CharacterDisplay    character_display;
          InteractivesDisplay interactives_display;
          PickupDisplay       pickup_display;
          ProjectileDisplay   projectile_display;

          SDL_Surface* bomb_sheet;
          SDL_Surface* attack_icon_sheet;
          SDL_Surface* player_heart_sheet;

          Bool  direction_keys [ Direction::count ];
          Bool  attack_key;
          Bool  item_key;
          Bool  switch_item_key;
          Bool  activate_key;

          Int32 player_spawn_tile_x;
          Int32 player_spawn_tile_y;

          Pickup::Type pickup_queue [ c_pickup_queue_size ];

          Stopwatch pickup_stopwatch;

#ifdef DEBUG
          Bool enemy_think;
          Bool invincible;
          Bool debug_text;
#endif
     };

     struct MemoryLocations {
          State*     state;

          Void calculate_locations ( GameMemory* memory );
     };
}

// exported functions to be called by the application
extern "C" Bool game_init       ( GameMemory&, Void* settings );
extern "C" Void game_destroy    ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update     ( GameMemory&, Real32 );
extern "C" Void game_render     ( GameMemory&, SDL_Surface* );

#endif

