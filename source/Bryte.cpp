#include "Bryte.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "Camera.hpp"
#include "MapDisplay.hpp"

using namespace bryte;

static const Real32 c_lever_width             = 0.5f;
static const Real32 c_lever_height            = 0.5f;

static const Char8* c_test_tilesheet_path        = "castle_tilesheet.bmp";
static const Char8* c_test_decorsheet_path       = "castle_decorsheet.bmp";
static const Char8* c_test_lampsheet_path        = "castle_lampsheet.bmp";
static const Char8* c_test_player_path           = "test_hero.bmp";
static const Char8* c_test_rat_path              = "test_rat.bmp";

const Real32 HealthPickup::c_dimension = 0.4f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

// assuming A attacks B
static Direction determine_damage_direction ( const Character& a, const Character& b, Random& random )
{
     Real32 a_center_x = a.position.x ( ) + a.width ( ) * 0.5f;
     Real32 a_center_y = a.position.y ( ) + a.collision_dimension.y ( ) * 0.5f;

     Real32 b_center_x = b.position.x ( ) + b.width ( ) * 0.5f;
     Real32 b_center_y = b.position.y ( ) + b.collision_dimension.x ( ) * 0.5f;

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

Bool State::initialize ( GameMemory& game_memory, Settings* settings )
{
     random.seed ( 13371 );

     player_spawn_tile_x = settings->player_spawn_tile_x;
     player_spawn_tile_y = settings->player_spawn_tile_y;

     player.position.set ( pixels_to_meters ( player_spawn_tile_x * Map::c_tile_dimension_in_pixels ),
                           pixels_to_meters ( player_spawn_tile_y * Map::c_tile_dimension_in_pixels ) );

     player.state  = Character::State::alive;
     player.facing = Direction::left;

     player.health           = 25;
     player.max_health       = 25;

     player.velocity.zero ( );

     player.dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
     player.collision_offset.set ( pixels_to_meters ( 5 ), pixels_to_meters ( 2 ) );
     player.collision_dimension.set ( pixels_to_meters ( 6 ), pixels_to_meters ( 7 ) );
     player.rotate_collision = false;

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

     // load test graphics
     if ( !load_bitmap_with_game_memory ( tilesheet, game_memory,
                                          c_test_tilesheet_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( decorsheet, game_memory,
                                          c_test_decorsheet_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( lampsheet, game_memory,
                                          c_test_lampsheet_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( rat_surface, game_memory,
                                          c_test_rat_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( player_surface, game_memory,
                                          c_test_player_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( interactives_display.interactive_sheets [ Interactive::Type::exit ],
                                          game_memory,
                                          "castle_exitsheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( interactives_display.interactive_sheets [ Interactive::Type::lever ],
                                          game_memory,
                                          "castle_leversheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( interactives_display.interactive_sheets [ Interactive::Type::pushable_block ],
                                          game_memory,
                                          "castle_pushableblocksheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( interactives_display.interactive_sheets [ Interactive::Type::torch ],
                                          game_memory,
                                          "castle_torchsheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( interactives_display.interactive_sheets [ Interactive::Type::pushable_torch ],
                                          game_memory,
                                          "castle_pushabletorchsheet.bmp" ) ) {
          return false;
     }

     map.load_master_list ( settings->map_master_list_filename );
     map.load_from_master_list ( settings->map_index, interactives );
     spawn_map_enemies ( );

     return true;
}

Void State::destroy ( )
{
     SDL_FreeSurface ( tilesheet );
     SDL_FreeSurface ( decorsheet );
     SDL_FreeSurface ( lampsheet );

     SDL_FreeSurface ( rat_surface );
     SDL_FreeSurface ( player_surface );
}

Bool State::spawn_enemy ( Real32 x, Real32 y )
{
     Enemy* enemy = nullptr;

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

     LOG_DEBUG ( "Spawning enemy at: %f, %f\n", x, y );

     enemy->state  = Character::State::alive;
     enemy->facing = Direction::left;
     enemy->type   = Enemy::Type::rat;

     enemy->health     = 3;
     enemy->max_health = 3;

     enemy->position.set ( x, y );

     enemy->velocity.zero ( );

     enemy->dimension.set ( pixels_to_meters ( 16 ), pixels_to_meters ( 16 ) );
     enemy->collision_offset.set ( pixels_to_meters ( 1 ), pixels_to_meters ( 4 ) );
     enemy->collision_dimension.set ( pixels_to_meters ( 14 ), pixels_to_meters ( 6 ) );
     enemy->rotate_collision = true;

     enemy->damage_pushed = Direction::left;

     enemy->state_watch.reset ( 0.0f );
     enemy->damage_watch.reset ( 0.0f );
     enemy->cooldown_watch.reset ( 0.0f );

     enemy_count++;

     return true;
}

Void State::spawn_map_enemies ( )
{
     for ( int i = 0; i < map.enemy_spawn_count ( ); ++i ) {
          auto& enemy_spawn = map.enemy_spawn ( i );

          spawn_enemy ( pixels_to_meters ( enemy_spawn.location.x * Map::c_tile_dimension_in_pixels ),
                        pixels_to_meters ( enemy_spawn.location.y * Map::c_tile_dimension_in_pixels ) );
     }
}

Void State::clear_enemies ( )
{
     for ( Uint32 i = 0; i < enemy_count; ++i ) {
          enemies [ i ].state = Character::State::dead;
     }

     enemy_count = 0;
}

Void State::player_death ( )
{
     player.state  = Character::State::alive;
     player.facing = Direction::left;

     player.health           = 25;
     player.max_health       = 25;

     player.position.set ( pixels_to_meters ( player_spawn_tile_x * Map::c_tile_dimension_in_pixels ),
                           pixels_to_meters ( player_spawn_tile_y * Map::c_tile_dimension_in_pixels ) );

     player.velocity.zero ( );

     player.damage_pushed = Direction::left;
     player.state_watch.reset ( 0.0f );
     player.damage_watch.reset ( 0.0f );
     player.cooldown_watch.reset ( 0.0f );

     // load the first map
     map.load_from_master_list ( 0, interactives );

     clear_enemies ( );
     spawn_map_enemies ( );
}

static Void render_character ( SDL_Surface* back_buffer, const Character& character,
                               SDL_Surface* character_surface,
                               Real32 camera_x, Real32 camera_y )
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

     SDL_Rect dest_rect = build_world_sdl_rect ( character.position.x ( ), character.position.y ( ),
                                                 character.width ( ), character.height ( ) );

     SDL_Rect clip_rect = {
          0, static_cast<Int32>( character.facing ) * Map::c_tile_dimension_in_pixels,
          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels
     };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( character_surface, &clip_rect, back_buffer, &dest_rect );
}

extern "C" Bool game_init ( GameMemory& game_memory, void* settings )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State );

     memory_locations->state = state;

     if ( !state->initialize ( game_memory, reinterpret_cast<Settings*>( settings ) ) ) {
          return false;
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
                    state->spawn_enemy ( state->player.position.x ( ) - state->player.width ( ),
                                         state->player.position.y ( ) );
               }
               break;
          }
     }
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     auto* state = get_state ( game_memory );

     if ( state->direction_keys [ Direction::up ] ) {
          state->player.walk ( Direction::up );
     }

     if ( state->direction_keys [ Direction::down ] ) {
          state->player.walk ( Direction::down );
     }

     if ( state->direction_keys [ Direction::right ] ) {
          state->player.walk ( Direction::right );
     }

     if ( state->direction_keys [ Direction::left ] ) {
          state->player.walk ( Direction::left );
     }

     if ( state->attack_key ) {
          state->player.attack ( );
     }

     state->player.update ( time_delta, state->map, state->interactives );

     for ( Uint32 i = 0; i < State::c_max_enemies; ++i ) {
          auto& enemy = state->enemies [ i ];

          if ( enemy.state == Character::State::dead ) {
               continue;
          }

          enemy.think ( state->player.position, state->random, time_delta );
          enemy.update ( time_delta, state->map, state->interactives );

          // check collision between player and enemy
          if ( state->player.state != Character::State::blinking &&
               state->player.collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( enemy, state->player, state->random );
               state->player.damage ( 1, damage_dir );

               if ( state->player.state == Character::State::dead ) {
                    state->player_death ( );
               }
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
                              health_pickup.position.set ( enemy.position.x ( ), enemy.position.y ( ) );
                              health_pickup.available = true;
                              break;
                         }
                    }
               }
          }
     }

     // update interactives
     state->interactives.update ( time_delta );

     if ( state->activate_key ) {
          state->activate_key = false;
          state->interactives.activate ( meters_to_pixels ( state->player.position.x ( ) ) /
                                         Map::c_tile_dimension_in_pixels,
                                         meters_to_pixels ( state->player.position.y ( ) ) /
                                         Map::c_tile_dimension_in_pixels);
     }

     for ( Uint32 i = 0; i < State::c_max_health_pickups; ++i ) {
          HealthPickup& health_pickup = state->health_pickups [ i ];

          if ( health_pickup.available ) {
               if ( rect_collides_with_rect ( state->player.position.x ( ), state->player.position.y ( ),
                                              state->player.width ( ), state->player.height ( ),
                                              health_pickup.position.x ( ), health_pickup.position.y ( ),
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
          auto& interactive = state->interactives.get_from_tile ( meters_to_pixels ( state->player.position.x ( ) ) /
                                                                  Map::c_tile_dimension_in_pixels,
                                                                  meters_to_pixels ( state->player.position.y ( ) ) /
                                                                  Map::c_tile_dimension_in_pixels );

          if ( interactive.type == Interactive::Type::exit &&
               interactive.interactive_exit.state == Exit::State::open ) {
               Vector new_position ( pixels_to_meters ( interactive.interactive_exit.exit_index_x * Map::c_tile_dimension_in_pixels ),
                                     pixels_to_meters ( interactive.interactive_exit.exit_index_y * Map::c_tile_dimension_in_pixels ) );

               map.load_from_master_list ( interactive.interactive_exit.map_index, state->interactives );

               state->clear_enemies ( );
               state->spawn_map_enemies ( );

               state->player.position = new_position;

               player_exit = map.position_to_tile_index ( state->player.position.x ( ),
                                                          state->player.position.y ( ) );
          }
     } else {
          auto player_tile_index = map.position_to_tile_index ( state->player.position.x ( ),
                                                                state->player.position.y ( ) );

          // clear the exit destination if they've left the tile
          if ( player_exit != player_tile_index ) {
               player_exit = 0;
          }
     }
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     auto* state = get_state ( game_memory );

     state->camera.set_x ( calculate_camera_position ( back_buffer->w, state->map.width ( ),
                                                       state->player.position.x ( ), state->player.width ( ) ) );

     state->camera.set_y ( calculate_camera_position ( back_buffer->h, state->map.height ( ),
                                                       state->player.position.y ( ), state->player.height ( ) ) );

     // draw map
     render_map ( back_buffer, state->tilesheet, state->map,
                  state->camera.x ( ), state->camera.y ( ) );
     render_map_decor ( back_buffer, state->decorsheet, state->map,
                        state->camera.x ( ), state->camera.y ( ) );
     render_map_lamps ( back_buffer, state->lampsheet, state->map,
                        state->camera.x ( ), state->camera.y ( ) );

     state->interactives_display.render_interactives ( back_buffer, state->interactives,
                                                       state->camera.x ( ), state->camera.y ( ) );

     state->map.reset_light ( );

     state->interactives_display.contribute_light ( state->interactives, state->map );

     Uint32 red     = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 green   = SDL_MapRGB ( back_buffer->format, 0, 255, 0 );
     Uint32 white   = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );

     // draw enemies
     for ( Uint32 i = 0; i < state->enemy_count; ++i ) {
          render_character ( back_buffer, state->enemies [ i ], state->rat_surface,
                             state->camera.x ( ), state->camera.y ( ) );
     }

     // draw player
     render_character ( back_buffer, state->player, state->player_surface,
                        state->camera.x ( ), state->camera.y ( ) );

     // draw player attack
     if ( state->player.state == Character::State::attacking ) {
          SDL_Rect attack_rect = build_world_sdl_rect ( state->player.attack_x ( ),
                                                        state->player.attack_y ( ),
                                                        state->player.attack_width ( ),
                                                        state->player.attack_height ( ) );

          world_to_sdl ( attack_rect, back_buffer, state->camera.x ( ), state->camera.y ( ) );

          SDL_FillRect ( back_buffer, &attack_rect, green );
     }

     for ( Uint32 i = 0; i < State::c_max_health_pickups; ++i ) {
          HealthPickup& health_pickup = state->health_pickups [ i ];

          if ( health_pickup.available ) {
               SDL_Rect health_pickup_rect = build_world_sdl_rect ( health_pickup.position.x ( ),
                                                                    health_pickup.position.y ( ),
                                                                    HealthPickup::c_dimension,
                                                                    HealthPickup::c_dimension );


               world_to_sdl ( health_pickup_rect, back_buffer, state->camera.x ( ), state->camera.y ( ) );

               SDL_FillRect ( back_buffer, &health_pickup_rect, red );
          }
     }

     render_light ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     // draw player health bar
     Real32 pct = static_cast<Real32>( state->player.health ) /
                  static_cast<Real32>( state->player.max_health );

     Int32 bar_len = static_cast<Int32>( 50.0f * pct );

     SDL_Rect health_bar_rect { 15, 15, bar_len, 10 };
     SDL_Rect health_bar_border_rect { 14, 14, 52, 12 };

     SDL_FillRect ( back_buffer, &health_bar_border_rect, white );
     SDL_FillRect ( back_buffer, &health_bar_rect, red );
}

