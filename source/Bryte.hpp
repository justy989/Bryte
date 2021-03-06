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
#include "Bomb.hpp"
#include "Dialogue.hpp"

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
     struct UITextMenu {
     public:

          Void init ( Int32 x, Int32 y );

          Bool add_option ( const Char8* text );

          Void render ( SDL_Surface* back_buffer, Text* text );

          Void next_option ( );
          Void prev_option ( );

     public:

          Int32 top_left_x;
          Int32 top_left_y;

          static const Int32 c_max_option_count = 8;
          static const Int32 c_max_option_length = 16;

          Char8 options [ c_max_option_count ] [ c_max_option_length ];
          Int32 option_count;

          Int32 selected;
     };

     struct Upgrade {
          Player::Upgrade type;
          Vector position;
     };

     struct Settings {
          Int32  region_index;

          Uint32 map_index;

          Int32  player_spawn_tile_x;
          Int32  player_spawn_tile_y;
     };

     struct State {
     public:

          enum GameState {
               intro,
               game,
               pause
          };

     public:

          Bool initialize ( GameMemory& game_memory, Settings* settings );
          Void destroy    ( );
          Void update ( GameMemory& game_memory, Real32 time_delta );
          Void handle_input ( GameMemory& game_memory, const GameInput& game_input );
          Void render ( GameMemory& game_memory, SDL_Surface* back_buffer );

          Void quit_game ( );

          Void update_intro ( GameMemory& game_memory, Real32 time_delta );
          Void update_game ( GameMemory& game_memory, Real32 time_delta );
          Void update_pause ( GameMemory& game_memory, Real32 time_delta );

          Void handle_intro_input ( GameMemory& game_memory, const GameInput& game_input );
          Void handle_game_input ( GameMemory& game_memory, const GameInput& game_input );
          Void handle_pause_input ( GameMemory& game_memory, const GameInput& game_input );

          Void render_intro ( GameMemory& game_memory, SDL_Surface* back_buffer );
          Void render_game ( GameMemory& game_memory, SDL_Surface* back_buffer );
          Void render_pause ( GameMemory& game_memory, SDL_Surface* back_buffer );

          Bool spawn_enemy ( const Vector& position, Uint8 id, Direction facing, Pickup::Type drop );
          Bool spawn_pickup ( const Vector& position, Pickup::Type type );
          Bool spawn_projectile ( Projectile::Type type, const Vector& position, Direction facing,
                                  Projectile::Alliance alliance );
          Bool spawn_bomb ( const Vector& position );

          Void start_game ( GameMemory& game_memory );
          Bool load_region ( GameMemory& game_memory );

          Void persist_map ( );
          Void spawn_map_enemies ( );

          Void tick_character_element ( Character& character );
          Void damage_character ( Character& character, Int32 amount, Direction direction );

          Void update_player ( GameMemory& game_memory, float time_delta );
          Void player_death ( );
          Bool check_player_block_projectile ( Projectile& projectile );

          Void update_enemies ( float time_delta );
          Void enemy_death ( const Enemy& enemy );

          Void push_interactive ( const Location& tile, Direction dir, const Map& map );

          Void update_interactives ( float time_delta );

          Void update_projectiles ( float time_delta );
          Void update_bombs ( float time_delta );
          Void update_pickups ( float time_delta );
          Void update_emitters ( float time_delta );
          Void update_light ( );

          Void setup_emitters_from_map_lamps ( );

          Void enqueue_pickup ( Pickup::Type type );
          Void dequeue_pickup ( );

          Void heal_enemies_in_range_of_fairy ( const Vector& position );

          Void change_map ( Int32 map_index, Bool persist = true );
          Direction player_on_border ( );

          Bool change_region ( GameMemory& game_memory, Int32 region_index );

          Void player_save ( );
          Void player_load ( );

          Void render_upgrade ( SDL_Surface* back_buffer );

     public:

          static const Int32 c_bomb_damage = 4;
          static const Int32 c_attack_damage = 1;
          static const Int32 c_block_damage = 0;
          static const Int32 c_enemy_damage = 1;
          static const Int32 c_burn_damage = 1;

          static const Int32 c_pickup_queue_size = 8;
          static const Real32 c_pickup_show_time;

     public:

          Settings* settings;

          GameState game_state;

          Random random;

          Player player;

          EntityManager<Enemy,        32> enemies;
          EntityManager<Pickup,        8> pickups;
          EntityManager<Projectile,   64> projectiles;
          EntityManager<Bomb,          8> bombs;
          EntityManager<Emitter,      32> emitters;

          Region       region;
          Map          map;
          Interactives interactives;
          Upgrade      upgrade;

          Dialogue dialogue;

          Vector       camera;

          Sound sound;

          MapDisplay          map_display;
          CharacterDisplay    character_display;
          InteractivesDisplay interactives_display;
          PickupDisplay       pickup_display;
          ProjectileDisplay   projectile_display;

          Text text;

          SDL_PixelFormat back_buffer_format;

          SDL_Surface* title_surface;

          SDL_Surface* bomb_sheet;
          SDL_Surface* player_heart_sheet;
          SDL_Surface* upgrade_sheet;

          UITextMenu slot_menu;
          UITextMenu pause_menu;

          Bool  direction_keys [ Direction::count ];
          Bool  attack_key;
          Bool  block_key;
          Bool  item_key;
          Bool  switch_item_key;
          Bool  activate_key;

          Int32 current_region;

          Location player_spawn_tile;

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

