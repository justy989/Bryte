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

#include "EntityManager.hpp"

#include "Random.hpp"

#include "Vector.hpp"
#include "Rect.hpp"

#include "MapDisplay.hpp"
#include "CharacterDisplay.hpp"
#include "InteractivesDisplay.hpp"

#include "Text.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     struct Arrow : public Entity {
          Void update ( float dt, const Map& map, Interactives& interactives );
          Bool check_for_solids ( const Map& map, Interactives& interactives );
          Void clear ( );

          Vector    position;
          Direction facing;
          Stopwatch stuck_watch;

          static Vector    collision_points [ Direction::count ];

          static const Real32 c_speed;
          static const Real32 c_stuck_time;
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
          Bool spawn_arrow ( const Vector& position, Direction facing );

          Void spawn_map_enemies ( );

          Void player_death ( );

     public:

          Random random;

          Player player;

          EntityManager<Enemy, 32> enemies;
          EntityManager<Pickup, 8> pickups;
          EntityManager<Arrow, 64> arrows;

          Map          map;
          Interactives interactives;

          Vector       camera;

          Text text;

          MapDisplay          map_display;
          CharacterDisplay    character_display;
          InteractivesDisplay interactives_display;

          SDL_Surface* pickup_sheet;
          SDL_Surface* arrow_sheet;

          SDL_Surface* attack_icon_sheet;

          Bool  direction_keys [ Direction::count ];
          Bool  attack_key;
          Bool  switch_attack_key;
          Bool  activate_key;

          Int32 player_spawn_tile_x;
          Int32 player_spawn_tile_y;

          Int32 player_key_count;

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
extern "C" Bool game_init       ( GameMemory&, void* settings );
extern "C" Void game_destroy    ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update     ( GameMemory&, Real32 );
extern "C" Void game_render     ( GameMemory&, SDL_Surface* );

#endif

