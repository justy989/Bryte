#include "Bryte.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "Camera.hpp"
#include "MapDisplay.hpp"

using namespace bryte;

static const Real32 c_player_speed            = 3.0f;

static const Real32 c_lever_width             = 0.5f;
static const Real32 c_lever_height            = 0.5f;
static const Real32 c_lever_activate_cooldown = 0.75f;

static const Char8* c_test_tilesheet_path     = "castle_tilesheet.bmp";

const Real32 HealthPickup::c_dimension = 0.4f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

// assuming A attacks B
static Direction determine_damage_direction ( const Character& a, const Character& b, Random& random )
{
     Real32 a_center_x = a.position_x + a.width * 0.5f;
     Real32 a_center_y = a.position_y + a.collision_height * 0.5f;

     Real32 b_center_x = b.position_x + b.width * 0.5f;
     Real32 b_center_y = b.position_y + b.collision_height * 0.5f;

     Real32 diff_x = b_center_x - a_center_x;
     Real32 diff_y = b_center_y - a_center_y;

     Real32 abs_x = fabs ( diff_x );
     Real32 abs_y = fabs ( diff_y );

     if ( abs_x > abs_y ) {
          if ( diff_x > 0.0f ) {
               return Direction::right;
          }

          return Direction::left;
     } else if ( abs_y > abs_x ) {
          if ( diff_y > 0.0f ) {
               return Direction::up;
          }

          return Direction::down;
     } else {
          Direction valid_dirs [ 2 ];

          if ( diff_x > 0.0f ) {
               valid_dirs [ 0 ] = Direction::right;
          } else {
               valid_dirs [ 0 ] = Direction::left;
          }

          if ( diff_y > 0.0f ) {
               valid_dirs [ 1 ] = Direction::up;
          } else {
               valid_dirs [ 1 ] = Direction::down;
          }

          // coin flip between using the x or y direction
          return valid_dirs [ random.generate ( 0, 2 ) ];
     }

     // the above cases should catch all
     ASSERT ( 0 );
     return Direction::left;
}

Bool State::initialize ( GameMemory& game_memory )
{
     random.seed ( 13371 );

     player.state  = Character::State::alive;
     player.facing = Direction::left;

     player.health           = 25;
     player.max_health       = 25;

     player.position_x       = Map::c_tile_dimension_in_meters * 2.0f;
     player.position_y       = Map::c_tile_dimension_in_meters * 2.0f;

     player.velocity_x       = 0.0f;
     player.velocity_y       = 0.0f;

     player.width            = 1.3f;
     player.height           = player.width * 1.5f;
     player.collision_height = player.width;

     player.damage_pushed = Direction::left;
     player.state_watch.reset ( 0.0f );
     player.damage_watch.reset ( 0.0f );
     player.cooldown_watch.reset ( 0.0f );

     // ensure all enemies start dead
     for ( Uint32 i = 0; i < c_max_enemies; ++i ) {
          enemies [ i ].state = Character::State::dead;
     }

     for ( Uint32 i = 0; i < c_max_health_pickups; ++i ) {
          health_pickups [ i ].available = false;
     }

     lever.position_x      = Map::c_tile_dimension_in_meters * 4.0f;
     lever.position_y      = Map::c_tile_dimension_in_meters * 7.5f - c_lever_width * 0.5f;
     lever.activate_tile_x = 3;
     lever.activate_tile_y = 8;
     lever.activate_time   = 0.0f;

     LOG_INFO ( "Loading tilesheet '%s'\n", c_test_tilesheet_path );

     FileContents bitmap_contents = load_entire_file ( c_test_tilesheet_path, &game_memory );
     tilesheet = load_bitmap ( &bitmap_contents );
     if ( !tilesheet ) {
          return false;
     }

     return true;
}

Void State::destroy ( )
{
     LOG_INFO ( "Freeing tilesheet: %s\n", c_test_tilesheet_path );
     SDL_FreeSurface ( tilesheet );
}

Bool State::spawn_enemy ( Real32 x, Real32 y )
{
     Character* enemy = nullptr;

     for ( Uint32 i = 0; i < c_max_enemies; ++i ) {
          if ( enemies [ i ].state == Character::State::dead ) {
               enemy = enemies + i;
               break;
          }
     }

     if ( !enemy ) {
          LOG_WARNING ( "Tried to spawn enemy when %d enemies already exist.\n", c_max_enemies );
          return false;
     }

     LOG_INFO ( "Spawning enemy at: %f, %f\n", x, y );

     enemy->state  = Character::State::alive;
     enemy->facing = Direction::left;

     enemy->health     = 3;
     enemy->max_health = 3;

     enemy->position_x = x;
     enemy->position_y = y;

     enemy->velocity_x = 0.0f;
     enemy->velocity_y = 0.0f;

     enemy->width            = 1.0f;
     enemy->height           = enemy->width * 1.5f;
     enemy->collision_height = enemy->width;

     enemy->damage_pushed = Direction::left;

     enemy->state_watch.reset ( 0.0f );
     enemy->damage_watch.reset ( 0.0f );
     enemy->cooldown_watch.reset ( 0.0f );

     enemy_count++;

     return true;
}

static Void render_character ( SDL_Surface* back_buffer, const Character& character,
                               Real32 camera_x, Real32 camera_y, Uint32 color )
{
     static const Int32 blink_length  = 4;
     static Bool        blink_on      = false;
     static Int32       blink_count   = 0;

     // do not draw if dead
     if ( character.state == Character::State::dead ) {
          return;
     }

     // update blinking
     if ( blink_count <= 0 ) {
          blink_count = blink_length;
          blink_on = !blink_on;
     } else {
          blink_count--;
     }

     if ( !blink_on && character.state == Character::State::blinking ) {
          return;
     }

     SDL_Rect character_rect = build_world_sdl_rect ( character.position_x, character.position_y,
                                                      character.width, character.height );

     world_to_sdl ( character_rect, back_buffer, camera_x, camera_y );

     SDL_FillRect ( back_buffer, &character_rect, color );
}

extern "C" Bool game_init ( GameMemory& game_memory, void* settings )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State );

     memory_locations->state = state;

     if ( !state->initialize ( game_memory ) ) {
          return false;
     }

     memory_locations->rooms = GAME_PUSH_MEMORY_ARRAY ( game_memory, Map::Room, Map::c_max_rooms );

     static const Uint8  c_map_1_width    = 20;
     static const Uint8  c_map_1_height   = 10;
     static const Uint8  c_map_2_width    = 11;
     static const Uint8  c_map_2_height   = 24;

     auto* rooms = memory_locations->rooms;

     rooms [ 0 ].initialize ( c_map_1_width, c_map_1_height,
                              GAME_PUSH_MEMORY_ARRAY ( game_memory, Map::Tile,
                                                       c_map_1_width * c_map_1_height ) );

     rooms [ 1 ].initialize ( c_map_2_width, c_map_2_height,
                              GAME_PUSH_MEMORY_ARRAY ( game_memory, Map::Tile,
                                                       c_map_2_width * c_map_2_height ) );

     rooms [ 0 ].exit_count = 1;

     rooms [ 0 ].exits [ 0 ].location_x    = 1;
     rooms [ 0 ].exits [ 0 ].location_y    = 8;
     rooms [ 0 ].exits [ 0 ].room_index    = 1;
     rooms [ 0 ].exits [ 0 ].destination_x = 9;
     rooms [ 0 ].exits [ 0 ].destination_y = 1;

     rooms [ 1 ].exit_count = 1;

     rooms [ 1 ].exits [ 0 ].location_x    = 9;
     rooms [ 1 ].exits [ 0 ].location_y    = 1;
     rooms [ 1 ].exits [ 0 ].room_index    = 0;
     rooms [ 1 ].exits [ 0 ].destination_x = 1;
     rooms [ 1 ].exits [ 0 ].destination_y = 8;

     state->map.set_current_room ( rooms + 0 );

     state->map.set_coordinate_value ( 1, 6, 4 );
     state->map.set_coordinate_solid ( 1, 6, true );

     state->map.set_coordinate_value ( 2, 6, 4 );
     state->map.set_coordinate_solid ( 2, 6, true );

     state->map.set_coordinate_value ( 3, 6, 13 );
     state->map.set_coordinate_solid ( 3, 6, true );

     state->map.set_coordinate_value ( 3, 8, 7 );
     state->map.set_coordinate_solid ( 3, 8, true );

     state->map.set_coordinate_value ( 3, 7, 7 );
     state->map.set_coordinate_solid ( 3, 7, true );

     for ( Uint32 i = 0; i < 2; ++i ) {
          Int32 max_tries = 10;
          Int32 random_tile_x = 0;
          Int32 random_tile_y = 0;

          while ( max_tries > 0 ) {
               random_tile_x = state->random.generate ( 0, 16 );
               random_tile_y = state->random.generate ( 0, 16 );

               if ( !state->map.is_position_solid ( random_tile_x, random_tile_y ) ) {
                    break;
               }

               max_tries--;
          }

          state->spawn_enemy ( Map::c_tile_dimension_in_meters * static_cast<Real32>( random_tile_x ),
                                    Map::c_tile_dimension_in_meters * static_cast<Real32>( random_tile_y ) );
     }

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{
     auto* state = get_state ( game_memory );

     state->destroy ( );
}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     auto* state = get_state ( game_memory );

     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_W:
               state->direction_keys [ Direction::up ]    = key_change.down;
               break;
          case SDL_SCANCODE_S:
               state->direction_keys [ Direction::down ]  = key_change.down;
               break;
          case SDL_SCANCODE_A:
               state->direction_keys [ Direction::left ]  = key_change.down;
               break;
          case SDL_SCANCODE_D:
               state->direction_keys [ Direction::right ] = key_change.down;
               break;
          case SDL_SCANCODE_C:
               state->attack_key = key_change.down;
               break;
          case SDL_SCANCODE_E:
               state->activate_key = key_change.down;
               break;
          case SDL_SCANCODE_8:
               if ( key_change.down ) {
                    state->spawn_enemy ( state->player.position_x - state->player.width,
                                              state->player.position_y );
               }
               break;
          }
     }
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     auto* state = get_state ( game_memory );
     auto* memory_locations = reinterpret_cast<MemoryLocations*>( game_memory.location ( ) );

     if ( state->direction_keys [ Direction::up ] ) {
          state->player.velocity_y = c_player_speed;
          state->player.facing     = Direction::up;
     }

     if ( state->direction_keys [ Direction::down ] ) {
          state->player.velocity_y = -c_player_speed;
          state->player.facing     = Direction::down;
     }

     if ( state->direction_keys [ Direction::right ] ) {
          state->player.velocity_x = c_player_speed;
          state->player.facing     = Direction::right;
     }

     if ( state->direction_keys [ Direction::left ] ) {
          state->player.velocity_x = -c_player_speed;
          state->player.facing     = Direction::left;
     }

     if ( state->attack_key ) {
          state->player.attack ( );
     }

     if ( state->activate_key ) {
          if ( state->lever.activate_time <= 0.0f ) {
               auto& player = state->player;
               auto& lever  = state->lever;

               if ( rect_collides_with_rect ( player.position_x, player.position_y,
                                              player.width, player.height,
                                              lever.position_x, lever.position_y,
                                              c_lever_width, c_lever_height ) ) {

                    auto tile_value = state->map.get_coordinate_value ( lever.activate_tile_x,
                                                                        lever.activate_tile_y );
                    auto tile_solid = state->map.get_coordinate_solid ( lever.activate_tile_x,
                                                                        lever.activate_tile_y );

                    Uint8 id    = tile_value ? 0 : 7;
                    Bool  solid = !tile_solid;

                    state->map.set_coordinate_value ( lever.activate_tile_x, lever.activate_tile_y, id );
                    state->map.set_coordinate_solid ( lever.activate_tile_x, lever.activate_tile_y, solid );

                    state->lever.activate_time = c_lever_activate_cooldown;
               }
          }
     }

     state->player.update ( time_delta, state->map );

     for ( Uint32 i = 0; i < State::c_max_enemies; ++i ) {
          auto& enemy = state->enemies [ i ];

          if ( enemy.state == Character::State::dead ) {
               continue;
          }

          enemy.update ( time_delta, state->map );

          // check collision between player and enemy
          if ( state->player.state != Character::State::blinking &&
               state->player.collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( enemy, state->player, state->random );
               state->player.damage ( 1, damage_dir );
          }

          // attacking enemy
          if ( state->player.state == Character::State::attacking &&
               enemy.state != Character::State::blinking &&
               state->player.attack_collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( state->player, enemy, state->random );
               enemy.damage ( 1, damage_dir );

               if ( enemy.state == Character::State::dead ) {
                    for ( Uint32 i = 0; i < State::c_max_health_pickups; ++i ) {
                         HealthPickup& health_pickup = state->health_pickups [ i ];

                         if ( !health_pickup.available ) {
                              health_pickup.position_x = enemy.position_x;
                              health_pickup.position_y = enemy.position_y;
                              health_pickup.available = true;
                              break;
                         }
                    }
               }
          }
     }

     for ( Uint32 i = 0; i < State::c_max_health_pickups; ++i ) {
          HealthPickup& health_pickup = state->health_pickups [ i ];

          if ( health_pickup.available ) {
               if ( rect_collides_with_rect ( state->player.position_x, state->player.position_y,
                                              state->player.width, state->player.height,
                                              health_pickup.position_x, health_pickup.position_y,
                                              HealthPickup::c_dimension, HealthPickup::c_dimension ) ) {
                    health_pickup.available    = false;
                    state->player.health += 5;

                    if ( state->player.health > state->player.max_health ) {
                         state->player.health = state->player.max_health;
                    }
               }
          }
     }

     auto& player_exit = state->player_exit_tile_index;
     auto& map         = state->map;

     // check if the player has exitted the area
     if ( player_exit == 0 ) {
          const auto* exit = map.check_position_exit ( state->player.position_x,
                                                       state->player.position_y );

          if ( exit ) {
               state->player.position_x = exit->destination_x * Map::c_tile_dimension_in_meters;
               state->player.position_y = exit->destination_y * Map::c_tile_dimension_in_meters;

               state->map.set_current_room ( &memory_locations->rooms [ exit->room_index ] );

               player_exit = map.position_to_tile_index ( state->player.position_x,
                                                          state->player.position_y );
          }
     } else {
          auto player_tile_index = map.position_to_tile_index ( state->player.position_x,
                                                                state->player.position_y );

          // clear the exit destination if they've left the tile
          if ( player_exit != player_tile_index ) {
               player_exit = 0;
          }
     }

     if ( state->lever.activate_time > 0.0f ) {
          state->lever.activate_time -= time_delta;
     }
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     auto* state = get_state ( game_memory );

     state->camera_x = calculate_camera_position ( back_buffer->w, state->map.width ( ),
                                                   state->player.position_x, state->player.width );

     state->camera_y = calculate_camera_position ( back_buffer->h, state->map.height ( ),
                                                   state->player.position_y, state->player.height );

     // draw map
     render_map ( back_buffer, state->tilesheet, state->map,
                  state->camera_x, state->camera_y );

     render_map_exits ( back_buffer, state->map,
                        state->camera_x, state->camera_y );

     Uint32 red     = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 blue    = SDL_MapRGB ( back_buffer->format, 0, 0, 255 );
     Uint32 green   = SDL_MapRGB ( back_buffer->format, 0, 255, 0 );
     Uint32 white   = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );
     Uint32 magenta = SDL_MapRGB ( back_buffer->format, 255, 0, 255 );

     // draw lever
     SDL_Rect lever_rect = build_world_sdl_rect ( state->lever.position_x,
                                                  state->lever.position_y,
                                                  c_lever_width, c_lever_height );

     world_to_sdl ( lever_rect, back_buffer, state->camera_x, state->camera_y );

     SDL_FillRect ( back_buffer, &lever_rect, magenta );

     // draw enemies
     for ( Uint32 i = 0; i < state->enemy_count; ++i ) {
          render_character ( back_buffer, state->enemies [ i ],
                             state->camera_x, state->camera_y, blue );
     }

     // draw player
     render_character ( back_buffer, state->player, state->camera_x, state->camera_y, red );

     // draw player attack
     if ( state->player.state == Character::State::attacking ) {
          SDL_Rect attack_rect = build_world_sdl_rect ( state->player.calc_attack_x ( ),
                                                        state->player.calc_attack_y ( ),
                                                        Character::c_attack_width,
                                                        Character::c_attack_height );

          // swap width and height for facing left and right
          if ( state->player.facing == Direction::left ||
               state->player.facing == Direction::right ) {
               attack_rect.w = meters_to_pixels ( Character::c_attack_height );
               attack_rect.h = meters_to_pixels ( Character::c_attack_width );
          }

          world_to_sdl ( attack_rect, back_buffer, state->camera_x, state->camera_y );

          SDL_FillRect ( back_buffer, &attack_rect, green );
     }

     for ( Uint32 i = 0; i < State::c_max_health_pickups; ++i ) {
          HealthPickup& health_pickup = state->health_pickups [ i ];

          if ( health_pickup.available ) {
               SDL_Rect health_pickup_rect = build_world_sdl_rect ( health_pickup.position_x,
                                                                    health_pickup.position_y,
                                                                    HealthPickup::c_dimension,
                                                                    HealthPickup::c_dimension );


               world_to_sdl ( health_pickup_rect, back_buffer, state->camera_x, state->camera_y );

               SDL_FillRect ( back_buffer, &health_pickup_rect, red );
          }
     }

     // draw player health bar
     Real32 pct = static_cast<Real32>( state->player.health ) /
                  static_cast<Real32>( state->player.max_health );

     Int32 bar_len = static_cast<Int32>( 50.0f * pct );

     SDL_Rect health_bar_rect { 15, 15, bar_len, 10 };
     SDL_Rect health_bar_border_rect { 14, 14, 52, 12 };

     SDL_FillRect ( back_buffer, &health_bar_border_rect, white );
     SDL_FillRect ( back_buffer, &health_bar_rect, red );
}

