#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

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
          Bool spawn_projectile ( Projectile::Type type, const Vector& position, Direction facing );
          Bool spawn_bomb ( const Vector& position );

          Void persist_map ( );
          Void spawn_map_enemies ( );

          Void player_death ( );

          Void enemy_death ( const Enemy& enemy );

          Void setup_emitters_from_map_lamps ( );

     public:

          Random random;

          Player player;

          EntityManager<Enemy,      32> enemies;
          EntityManager<Pickup,      8> pickups;
          EntityManager<Projectile, 64> projectiles;
          EntityManager<Bomb,        8> bombs;
          EntityManager<Emitter,    32> emitters;

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

          SDL_Surface* arrow_sheet;
          SDL_Surface* bomb_sheet;

          SDL_Surface* attack_icon_sheet;

          Bool  direction_keys [ Direction::count ];
          Bool  attack_key;
          Bool  switch_attack_key;
          Bool  activate_key;

          Int32 player_spawn_tile_x;
          Int32 player_spawn_tile_y;

          Int32 player_key_count;

          Stopwatch player_deathwatch;

#ifdef DEBUG
          Bool enemy_think;
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

