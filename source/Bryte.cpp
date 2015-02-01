#include "Bryte.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "Camera.hpp"
#include "MapDisplay.hpp"

#ifdef WIN32
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cmath>
#include <cstdlib>

using namespace bryte;

static const Real32 c_lever_width             = 0.5f;
static const Real32 c_lever_height            = 0.5f;

static const Char8* c_test_tilesheet_path        = "castle_tilesheet.bmp";
static const Char8* c_test_decorsheet_path       = "castle_decorsheet.bmp";
static const Char8* c_test_lampsheet_path        = "castle_lampsheet.bmp";
static const Char8* c_test_player_path           = "test_hero.bmp";
static const Char8* c_test_rat_path              = "test_rat.bmp";
static const Char8* c_test_bat_path              = "test_bat.bmp";
static const Char8* c_test_pickups_path          = "test_pickups.bmp";

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

Vector vector_from_direction ( Direction dir )
{
     switch ( dir )
     {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          return Vector { -1.0f, 0.0f };
     case Direction::up:
          return Vector { 0.0f, 1.0f };
     case Direction::right:
          return Vector { 1.0f, 0.0f };
     case Direction::down:
          return Vector { 0.0f, -1.0f };
     }

     // should not hit
     return Vector { 0.0f, 0.0f };
}

const Real32 Arrow::c_speed = 20.0f;
const Real32 Arrow::c_stuck_time = 1.5f;

Vector Arrow::collision_points [ Direction::count ];

Bool Arrow::check_for_solids ( const Map& map, Interactives& interactives )
{
     Vector arrow_center = position + Arrow::collision_points [ facing ];

     Int32 tile_x = static_cast<Int32>( arrow_center.x ( ) / Map::c_tile_dimension_in_meters );
     Int32 tile_y = static_cast<Int32>( arrow_center.y ( ) / Map::c_tile_dimension_in_meters );

     if ( tile_x < 0 || tile_x >= map.width ( ) ||
          tile_y < 0 || tile_y >= map.height ( ) ) {
          return false;
     }

     if ( map.get_coordinate_solid ( tile_x, tile_y ) ) {
          return true;
     }

     Auto& interactive = interactives.get_from_tile ( tile_x, tile_y );

     if ( interactive.is_solid ( ) ) {
          // do not activate exits!
          if ( interactive.type != Interactive::Type::exit ) {
               interactive.activate ( interactives );
          }
          return true;
     } else {
          if ( interactive.type == Interactive::Type::exit ) {
               // otherwise arrows can escape when doors are open
               // TODO: is this ok?
               return true;
          }
     }

     return false;
}

Void Arrow::update ( float time_delta, const Map& map, Interactives& interactives )
{
     switch ( life_state ) {
     default:
          ASSERT ( 0 );
          break;
     case LifeState::dead:
          break;
     case LifeState::spawning:
          // nop for now ** pre-mature planning wooo **
          life_state = LifeState::alive;
          break;
     case LifeState::alive:
          if ( stuck_watch.expired ( ) ) {
               position += vector_from_direction ( facing ) * c_speed * time_delta;

               if ( check_for_solids ( map, interactives ) ) {
                    stuck_watch.reset ( c_stuck_time );
               }
          } else {
               stuck_watch.tick ( time_delta );

               if ( track_entity.entity ) {
                    position = track_entity.entity->position + track_entity.offset;
               }

               if ( stuck_watch.expired ( ) ) {
                    life_state = dead;
                    position.zero ( );
                    clear ( );
               }
          }
          break;
     }
}

Void Arrow::clear ( )
{
     facing = Direction::left;
     stuck_watch.reset ( 0.0f );
     track_entity.entity = nullptr;
     track_entity.offset.zero ( );
}

const Real32 Bomb::c_explode_time = 3.0f;
const Real32 Bomb::c_explode_radius = Map::c_tile_dimension_in_meters * 2.0f;

Void Bomb::update ( float dt )
{
     // allow 1 frame where the bomb has expired but isn't dead
     if ( explode_watch.expired ( ) ) {
          life_state = Entity::LifeState::dead;
     }

     explode_watch.tick ( dt );
}

Void Bomb::clear ( )
{
     explode_watch.reset ( 0.0f );
}

// assuming A attacks B
static Direction determine_damage_direction ( const Vector& a, const Vector& b, Random& random )
{
     Vector diff = b - a;

     Real32 abs_x = fabs ( diff.x ( ) );
     Real32 abs_y = fabs ( diff.y ( ) );

     if ( abs_x > abs_y ) {
          if ( diff.x ( ) > 0.0f ) {
               return Direction::right;
          }

          return Direction::left;
     } else if ( abs_y > abs_x ) {
          if ( diff.y ( ) > 0.0f ) {
               return Direction::up;
          }

          return Direction::down;
     } else {
          Direction valid_dirs [ 2 ];

          if ( diff.x ( ) > 0.0f ) {
               valid_dirs [ 0 ] = Direction::right;
          } else {
               valid_dirs [ 0 ] = Direction::left;
          }

          if ( diff.y ( ) > 0.0f ) {
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

     player.clear ( );

     player.position.set ( pixels_to_meters ( player_spawn_tile_x * Map::c_tile_dimension_in_pixels ),
                           pixels_to_meters ( player_spawn_tile_y * Map::c_tile_dimension_in_pixels ) );

     player.life_state = Entity::LifeState::alive;

     for ( int i = 0; i < Enemy::Type::count; ++i ) {
          character_display.enemy_sheets [ i ] = nullptr;
     }

     interactives_display.interactive_sheet = nullptr;

     FileContents text_contents = load_entire_file ( "text.bmp", &game_memory );

     text.fontsheet         = load_bitmap ( &text_contents );
     text.character_width   = 5;
     text.character_height  = 8;
     text.character_spacing = 1;

     if ( !text.fontsheet ) {
          return false;
     }

     // load test graphics
     if ( !load_bitmap_with_game_memory ( map_display.tilesheet, game_memory,
                                          c_test_tilesheet_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( map_display.decorsheet, game_memory,
                                          c_test_decorsheet_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( map_display.lampsheet, game_memory,
                                          c_test_lampsheet_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( character_display.enemy_sheets [ Enemy::Type::rat ], game_memory,
                                          c_test_rat_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( character_display.enemy_sheets [ Enemy::Type::bat ], game_memory,
                                          c_test_bat_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( character_display.player_sheet, game_memory,
                                          c_test_player_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( character_display.vertical_sword_sheet, game_memory,
                                          "test_vertical_sword.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( character_display.horizontal_sword_sheet, game_memory,
                                          "test_horizontal_sword.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( interactives_display.interactive_sheet,
                                          game_memory,
                                          "castle_interactivesheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( pickup_sheet, game_memory, c_test_pickups_path ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( arrow_sheet, game_memory, "test_arrow.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( bomb_sheet, game_memory, "test_bomb.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( attack_icon_sheet, game_memory, "test_attack_icon.bmp" ) ) {
          return false;
     }

     for ( Int32 i = 0; i < 4; ++i ) {
          direction_keys [ i ] = false;
     }

     pickups.clear ( );
     arrows.clear ( );
     emitters.clear ( );
     enemies.clear ( );

     map.load_master_list ( settings->map_master_list_filename );
     map.load_from_master_list ( settings->map_index, interactives );
     spawn_map_enemies ( );

     setup_emitters_from_map_lamps ( );

     attack_key = false;
     switch_attack_key = false;

     Arrow::collision_points [ Direction::left ].set ( pixels_to_meters ( 1 ), pixels_to_meters ( 7 ) );
     Arrow::collision_points [ Direction::up ].set ( pixels_to_meters ( 7 ), pixels_to_meters ( 14 ) );
     Arrow::collision_points [ Direction::right ].set ( pixels_to_meters ( 14 ), pixels_to_meters ( 7 ) );
     Arrow::collision_points [ Direction::down ].set ( pixels_to_meters ( 7 ), pixels_to_meters ( 1 ) );

#ifdef DEBUG
     enemy_think = true;
#endif

     return true;
}

Void State::destroy ( )
{
     SDL_FreeSurface ( map_display.tilesheet );
     SDL_FreeSurface ( map_display.decorsheet );
     SDL_FreeSurface ( map_display.lampsheet );

     SDL_FreeSurface ( character_display.player_sheet );

     for ( int i = 0; i < Enemy::Type::count; ++i ) {
          SDL_FreeSurface ( character_display.enemy_sheets [ i ] );
     }

     SDL_FreeSurface ( interactives_display.interactive_sheet );

     SDL_FreeSurface ( pickup_sheet );
     SDL_FreeSurface ( arrow_sheet );

     SDL_FreeSurface ( attack_icon_sheet );
}

Bool State::spawn_enemy ( const Vector& position, Uint8 id, Direction facing, Pickup::Type drop )
{
     Enemy* enemy = enemies.spawn ( position );

     if ( !enemy ) {
          return false;
     }

     enemy->init ( static_cast<Enemy::Type>( id ), position.x ( ), position.y ( ), facing, drop );

#ifdef DEBUG
     static const Char8* enemy_id_names [ ] = { "rat", "bat" };
#endif

     LOG_DEBUG ( "Spawning enemy %s at: %f, %f\n", enemy_id_names [ id ], position.x ( ), position.y ( ) );

     return true;
}

Bool State::spawn_pickup ( const Vector& position, Pickup::Type type )
{
     Auto* pickup = pickups.spawn ( position );

     if ( !pickup ) {
          return false;
     }

     pickup->type = type;

     LOG_DEBUG ( "Spawn pickup %s at %f, %f\n", Pickup::c_names [ type ], position.x ( ), position.y ( ) );

     return true;
}

bool State::spawn_arrow ( const Vector& position, Direction facing )
{
     Auto* arrow = arrows.spawn ( position );

     if ( !arrow ) {
          return false;
     }

     arrow->facing = facing;

     LOG_DEBUG ( "spawning arrow at %f, %f\n", position.x ( ), position.y ( ) );

     Auto* emitter = emitters.spawn ( position );

     if ( emitter ) {
          emitter->setup_to_track_entity ( arrow, Arrow::collision_points [ facing ],
                                           SDL_MapRGB ( pickup_sheet->format, 255, 255, 255 ),
                                           0.0f, 0.0f, 0.5f, 0.5f, 0.1f, 0.1f, 1, 2 );
     }

     return true;
}

Bool State::spawn_bomb ( const Vector& position )
{
     Auto* bomb = bombs.spawn ( position );

     if ( !bomb ) {
          return false;
     }

     bomb->explode_watch.reset ( Bomb::c_explode_time );

     LOG_DEBUG ( "spawning bomb at %f, %f\n", position.x ( ), position.y ( ) );

     Auto* emitter = emitters.spawn ( position );

     if ( emitter ) {
          Vector offset { Map::c_tile_dimension_in_meters * 0.5f, Map::c_tile_dimension_in_meters };
          emitter->setup_to_track_entity ( bomb, offset,
                                           SDL_MapRGB ( pickup_sheet->format, 255, 255, 255 ),
                                           0.785f, 2.356f, 1.0f, 1.0f, 0.5f, 0.5f, 1, 10 );
     }

     return true;
}

Void State::spawn_map_enemies ( )
{
     for ( int i = 0; i < map.enemy_spawn_count ( ); ++i ) {
          Auto& enemy_spawn = map.enemy_spawn ( i );

          Vector position { pixels_to_meters ( enemy_spawn.location.x * Map::c_tile_dimension_in_pixels ),
                            pixels_to_meters ( enemy_spawn.location.y * Map::c_tile_dimension_in_pixels ) };

          spawn_enemy ( position, enemy_spawn.id, enemy_spawn.facing, enemy_spawn.drop );
     }
}

Void State::player_death ( )
{
     player.clear ( );

     player.life_state = Entity::LifeState::alive;

     player.position.set ( pixels_to_meters ( player_spawn_tile_x * Map::c_tile_dimension_in_pixels ),
                           pixels_to_meters ( player_spawn_tile_y * Map::c_tile_dimension_in_pixels ) );

     // load the first map
     map.load_from_master_list ( 0, interactives );

     enemies.clear ( );
     spawn_map_enemies ( );
}

Void State::drop_item_on_enemy_death ( const Enemy& enemy )
{
     if ( enemy.is_dead ( ) ) {
          if ( enemy.drop != Pickup::Type::none ) {
               spawn_pickup ( enemy.position, enemy.drop );
          }
#if 0
          // generate an item to drop
          Auto roll = state->random.generate ( 1, 11 );

          if ( roll > 5 && roll < 8 ) {
               state->spawn_pickup ( enemy.position.x ( ), enemy.position.y ( ), Pickup::Type::health );
          } else if ( roll >= 8 ) {
               state->spawn_pickup ( enemy.position.x ( ), enemy.position.y ( ), Pickup::Type::key );
          }
#endif
     }
}

Void State::setup_emitters_from_map_lamps ( )
{
     for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
          Auto& lamp = map.lamp ( i );

          Vector position { pixels_to_meters ( lamp.location.x * Map::c_tile_dimension_in_pixels ),
                            pixels_to_meters ( lamp.location.y * Map::c_tile_dimension_in_pixels ) };
          Vector offset { Map::c_tile_dimension_in_meters * 0.4f,
                          Map::c_tile_dimension_in_meters * 0.7f };

          Auto* emitter = emitters.spawn ( position );

          if ( !emitter ) {
               break;
          }

          emitter->setup_immortal ( position + offset, SDL_MapRGB ( pickup_sheet->format, 255, 255, 0 ),
                                    0.78f, 2.35f, 0.5f, 0.75f, 0.5f, 1.0f, 1, 10 );
     }
}

extern "C" Bool game_init ( GameMemory& game_memory, Void* settings )
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
     Auto* state = get_state ( game_memory );

     state->destroy ( );
}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     Auto* state = get_state ( game_memory );

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
          case SDL_SCANCODE_Q:
               state->switch_attack_key = key_change.down;
               break;
          case SDL_SCANCODE_SPACE:
               state->attack_key = key_change.down;
               break;
          case SDL_SCANCODE_E:
               state->activate_key = key_change.down;
               break;
          case SDL_SCANCODE_8:
               if ( key_change.down ) {
                    Vector spawn_pos = state->player.position - Vector ( Map::c_tile_dimension_in_meters, 0.0f );
                    state->spawn_enemy ( spawn_pos, 0, state->player.facing, Pickup::Type::health );
               }
               break;
#ifdef DEBUG
          case SDL_SCANCODE_I:
               if ( key_change.down ) {
                    state->enemy_think = !state->enemy_think;
               }
               break;
          case SDL_SCANCODE_K:
               if ( key_change.down ) {
                    state->player.key_count++;
               }
               break;
          case SDL_SCANCODE_O:
               if ( key_change.down ) {
                    state->player.arrow_count++;
               }
               break;
          case SDL_SCANCODE_B:
               if ( key_change.down ) {
                    state->player.bomb_count++;
               }
               break;
#endif
          }
     }
}

Void character_adjacent_tile ( const Character& character, Int32* adjacent_tile_x, Int32* adjacent_tile_y )
{
     Vector character_center { character.collision_center_x ( ),
                               character.collision_center_y ( ) };

     Int32 character_center_tile_x = meters_to_pixels ( character_center.x ( ) ) / Map::c_tile_dimension_in_pixels;
     Int32 character_center_tile_y = meters_to_pixels ( character_center.y ( ) ) / Map::c_tile_dimension_in_pixels;

     switch ( character.facing ) {
          default:
               break;
          case Direction::left:
               character_center_tile_x--;
               break;
          case Direction::right:
               character_center_tile_x++;
               break;
          case Direction::up:
               character_center_tile_y++;
               break;
          case Direction::down:
               character_center_tile_y--;
               break;
     }

     *adjacent_tile_x = character_center_tile_x;
     *adjacent_tile_y = character_center_tile_y;
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     Auto* state = get_state ( game_memory );

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

     if ( state->switch_attack_key ) {
          state->switch_attack_key = false;

          Int32 new_attack_mode = ( static_cast<Int32>( state->player.attack_mode ) + 1 ) %
                                  Player::AttackMode::count;
          state->player.attack_mode = static_cast<Player::AttackMode>( new_attack_mode );
     }

     if ( state->attack_key ) {
          state->attack_key = false;

          switch ( state->player.attack_mode ) {
          default:
               ASSERT ( 0 );
               break;
          case Player::AttackMode::sword:
               state->player.attack ( );
               break;
          case Player::AttackMode::arrow:
               if ( state->player.arrow_count > 0 ) {
                    state->spawn_arrow ( state->player.position, state->player.facing );
                    state->player.arrow_count--;
               }
               break;
          case Player::AttackMode::bomb:
               if ( state->player.bomb_count > 0 ) {
                    state->spawn_bomb ( state->player.position );
                    state->player.bomb_count--;
               }
               break;
          }
     }

     state->player.update ( time_delta, state->map, state->interactives );

     if ( state->player.state == Character::State::pushing ) {
          Int32 push_tile_x = 0;
          Int32 push_tile_y = 0;

          character_adjacent_tile ( state->player, &push_tile_x, &push_tile_y );

          if ( push_tile_x >= 0 && push_tile_x < state->interactives.width ( ) &&
               push_tile_y >= 0 && push_tile_y < state->interactives.height ( ) ) {
               state->interactives.push ( push_tile_x, push_tile_y, state->player.facing, state->map );
          }
     }

     for ( Uint32 i = 0; i < state->enemies.max ( ); ++i ) {
          Auto& enemy = state->enemies [ i ];

          if ( enemy.is_dead ( ) ) {
               continue;
          }

#ifdef DEBUG
          if ( state->enemy_think ) {
               enemy.think ( state->player.position, state->random, time_delta );
          }
#else
          enemy.think ( state->player.position, state->random, time_delta );
#endif

          enemy.update ( time_delta, state->map, state->interactives );

#if 0
          // not sure I want enemies messing with your puzzles
          if ( enemy.state == Character::State::pushing ) {
               Int32 push_tile_x = 0;
               Int32 push_tile_y = 0;

               character_adjacent_tile ( enemy, &push_tile_x, &push_tile_y );

               if ( push_tile_x >= 0 && push_tile_x < state->interactives.width ( ) &&
                    push_tile_y >= 0 && push_tile_y < state->interactives.height ( ) ) {
                    state->interactives.push ( push_tile_x, push_tile_y, enemy.facing, state->map, state->enemies, state->enemy_count, state->player );
               }
          }
#endif

          // check collision between player and enemy
          if ( state->player.state != Character::State::blinking &&
               state->player.collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( enemy.collision_center ( ),
                                                                   state->player.collision_center ( ),
                                                                   state->random );
               state->player.damage ( 1, damage_dir );

               if ( state->player.life_state == Entity::LifeState::dead ) {
                    state->player_death ( );
               }
          }

          // attacking enemy
          if ( state->player.state == Character::State::attacking &&
               enemy.state != Character::State::blinking &&
               state->player.attack_collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( state->player.collision_center ( ),
                                                                   enemy.collision_center ( ),
                                                                   state->random );
               enemy.damage ( 1, damage_dir );
               state->drop_item_on_enemy_death ( enemy );
          }
     }

     // update interactives
     state->interactives.update ( time_delta );

     if ( state->activate_key ) {
          state->activate_key = false;

          Int32 player_activate_tile_x = 0;
          Int32 player_activate_tile_y = 0;

          character_adjacent_tile ( state->player, &player_activate_tile_x, &player_activate_tile_y );

          if ( player_activate_tile_x >= 0 && player_activate_tile_x < state->interactives.width ( ) &&
               player_activate_tile_y >= 0 && player_activate_tile_y < state->interactives.height ( ) ) {

               Auto& interactive = state->interactives.get_from_tile ( player_activate_tile_x, player_activate_tile_y );

               if ( interactive.type == Interactive::Type::exit ) {
                    if ( interactive.interactive_exit.state == Exit::State::locked &&
                         state->player.key_count > 0 ) {
                         LOG_DEBUG ( "Unlock Door: %d, %d\n", player_activate_tile_x, player_activate_tile_y );
                         state->interactives.activate ( player_activate_tile_x, player_activate_tile_y );
                         state->player_key_count--;
                    }
               } else {
                    LOG_DEBUG ( "Activate: %d, %d\n", player_activate_tile_x, player_activate_tile_y );
                    state->interactives.activate ( player_activate_tile_x, player_activate_tile_y );
               }
          }

     }

     for ( Uint32 i = 0; i < state->arrows.max ( ); ++i ) {
          Auto& arrow = state->arrows [ i ];

          if ( arrow.is_dead ( ) ) {
               continue;
          }

          arrow.update ( time_delta, state->map, state->interactives );

          if ( !arrow.stuck_watch.expired ( ) ) {
               continue;
          }

          Vector arrow_collision_point = arrow.position + arrow.collision_points [ arrow.facing ];

          for ( Uint32 c = 0; c < state->enemies.max ( ); ++c ) {
               Auto& enemy = state->enemies [ c ];

               if ( enemy.is_dead ( ) ) {
                    continue;
               }

               if ( point_inside_rect ( arrow_collision_point.x ( ),
                                        arrow_collision_point.y ( ),
                                        enemy.collision_x ( ), enemy.collision_y ( ),
                                        enemy.collision_x ( ) + enemy.collision_width ( ),
                                        enemy.collision_y ( ) + enemy.collision_height ( ) ) ) {
                    enemy.damage ( 1, arrow.facing );
                    state->drop_item_on_enemy_death ( enemy );
                    arrow.track_entity.entity = &enemy;
                    arrow.track_entity.offset = arrow.position - enemy.position;
                    arrow.stuck_watch.reset ( Arrow::c_stuck_time );
                    break;
               }
          }
     }

     for ( Uint32 i = 0; i < state->bombs.max ( ); ++i ) {
          Auto& bomb = state->bombs [ i ];

          if ( bomb.is_dead ( ) ) {
               continue;
          }

          bomb.update ( time_delta );

          if ( bomb.explode_watch.expired ( ) ) {
               // damage nearby enemies
               for ( Uint32 c = 0; c < state->enemies.max ( ); ++c ) {
                    Auto& enemy = state->enemies [ c ];

                    if ( enemy.is_dead ( ) ) {
                         continue;
                    }

                    if ( enemy.collision_center ( ).distance_to ( bomb.position ) < Bomb::c_explode_radius ) {
                         enemy.damage ( 5, determine_damage_direction ( bomb.position,
                                                                        enemy.collision_center ( ),
                                                                        state->random ) );
                         state->drop_item_on_enemy_death ( enemy );
                    }
               }

               // activate nearby objects
               Vector bomb_center { bomb.position.x ( ) + Map::c_tile_dimension_in_meters * 0.5f,
                                    bomb.position.y ( ) + Map::c_tile_dimension_in_meters * 0.5f };
               Int32 tile_radius = meters_to_pixels ( Bomb::c_explode_radius ) / Map::c_tile_dimension_in_pixels;
               Int32 tile_min_x = meters_to_pixels ( bomb_center.x ( ) ) / Map::c_tile_dimension_in_pixels;
               Int32 tile_min_y = meters_to_pixels ( bomb_center.y ( ) ) / Map::c_tile_dimension_in_pixels;

               tile_min_x -= tile_radius;
               tile_min_y -= tile_radius;

               Int32 tile_max_x = tile_min_x + tile_radius * 2;
               Int32 tile_max_y = tile_min_y + tile_radius * 2;

               CLAMP ( tile_min_x, 0, state->interactives.width ( ) - 1 );
               CLAMP ( tile_min_y, 0, state->interactives.height ( ) - 1 );
               CLAMP ( tile_max_x, 0, state->interactives.width ( ) - 1 );
               CLAMP ( tile_max_y, 0, state->interactives.height ( ) - 1 );

               for ( Int32 y = tile_min_y; y <= tile_max_y; ++y ) {
                    for ( Int32 x = tile_min_x; x <= tile_max_x; ++x ) {
                         state->interactives.activate ( x, y );
                    }
               }

               // create quick emitter
               Auto* emitter = state->emitters.spawn ( bomb.position );

               if ( emitter ) {
                    Vector offset { Map::c_tile_dimension_in_meters * 0.5f,
                                    Map::c_tile_dimension_in_meters * 0.5f };
                    emitter->setup_limited_time ( bomb.position + offset, 0.5f,
                                                  SDL_MapRGB ( state->pickup_sheet->format, 200, 200, 200 ),
                                                  0.0f, 6.28f, 0.5f, 0.5f, 6.0f, 6.0f,
                                                  Emitter::c_max_particles, 0 );
               }
          }
     }

     for ( Uint32 i = 0; i < state->pickups.max ( ); ++i ) {
          Pickup& pickup = state->pickups [ i ];

          if ( pickup.is_dead ( ) ) {
               continue;
          }

          if ( rect_collides_with_rect ( state->player.collision_x ( ), state->player.collision_y ( ),
                                         state->player.collision_width ( ), state->player.collision_height ( ),
                                         pickup.position.x ( ), pickup.position.y ( ),
                                         Pickup::c_dimension_in_meters, Pickup::c_dimension_in_meters ) ) {

               LOG_DEBUG ( "Player got pickup %s\n", Pickup::c_names [ pickup.type ] );

               switch ( pickup.type ) {
               default:
                    break;
               case Pickup::Type::health:
                    state->player.health += 5;

                    if ( state->player.health > state->player.max_health ) {
                         state->player.health = state->player.max_health;
                    }
                    break;
               case Pickup::Type::key:
                    state->player.key_count++;
                    break;
               case Pickup::Type::arrow:
                    state->player.arrow_count++;
                    break;
               }

               pickup.type = Pickup::Type::none;
               pickup.life_state = Entity::LifeState::dead;
          }
     }

     for ( Uint32 i = 0; i < state->emitters.max ( ); ++i ) {
          Auto& emitter = state->emitters [ i ];
          if ( emitter.is_dead ( ) ) {
               continue;
          }

          emitter.update ( time_delta, state->random );
     }

     Auto& map         = state->map;

     Vector player_center { state->player.collision_center_x ( ),
                            state->player.collision_center_y ( ) };

     Int32 player_center_tile_x = meters_to_pixels ( player_center.x ( ) ) / Map::c_tile_dimension_in_pixels;
     Int32 player_center_tile_y = meters_to_pixels ( player_center.y ( ) ) / Map::c_tile_dimension_in_pixels;

     if ( player_center_tile_x >= 0 && player_center_tile_x < state->interactives.width ( ) &&
          player_center_tile_y >= 0 && player_center_tile_y < state->interactives.height ( ) ) {

          Auto& interactive = state->interactives.get_from_tile ( player_center_tile_x, player_center_tile_y );

          if ( interactive.type == Interactive::Type::exit &&
               interactive.interactive_exit.state == Exit::State::open &&
               interactive.interactive_exit.direction == opposite_direction ( state->player.facing ) ) {
               Vector new_position ( pixels_to_meters ( interactive.interactive_exit.exit_index_x *
                                                        Map::c_tile_dimension_in_pixels ),
                                     pixels_to_meters ( interactive.interactive_exit.exit_index_y *
                                                        Map::c_tile_dimension_in_pixels ) );

               new_position += Vector ( Map::c_tile_dimension_in_meters * 0.5f,
                                        Map::c_tile_dimension_in_meters * 0.5f );

               map.load_from_master_list ( interactive.interactive_exit.map_index, state->interactives );

               state->pickups.clear ( );
               state->arrows.clear ( );
               state->enemies.clear ( );
               state->emitters.clear ( );
               state->spawn_map_enemies ( );

               state->setup_emitters_from_map_lamps ( );

               state->player.set_collision_center ( new_position.x ( ), new_position.y ( ) );

               LOG_DEBUG ( "Teleporting player to %f %f on new map\n",
                           state->player.position.x ( ),
                           state->player.position.y ( ) );
          }
     }

     state->map.reset_light ( );
     state->interactives.contribute_light ( map );

     // give interactives their light values
     for ( Int32 y = 0; y < state->interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < state->interactives.width ( ); ++x ) {
               state->interactives.light ( x, y, map.get_coordinate_light ( x, y ) );
          }
     }
}

static Void render_pickup ( SDL_Surface* back_buffer, SDL_Surface* pickup_sheet, Pickup& pickup,
                            Real32 camera_x, Real32 camera_y )
{
     if ( pickup.type == Pickup::Type::none ||
          pickup.type == Pickup::Type::ingredient ) {
          return;
     }

     SDL_Rect dest_rect = build_world_sdl_rect ( pickup.position.x ( ), pickup.position.y ( ),
                                                 Pickup::c_dimension_in_meters,
                                                 Pickup::c_dimension_in_meters );

     SDL_Rect clip_rect { ( static_cast<Int32>( pickup.type ) - 1) * Pickup::c_dimension_in_pixels, 0,
                          Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( pickup_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_arrow ( SDL_Surface* back_buffer, SDL_Surface* arrow_sheet, const Arrow& arrow,
                           Real32 camera_x, Real32 camera_y )
{

     SDL_Rect dest_rect = build_world_sdl_rect ( arrow.position.x ( ), arrow.position.y ( ),
                                                 Map::c_tile_dimension_in_meters,
                                                 Map::c_tile_dimension_in_meters );

     SDL_Rect clip_rect { 0, static_cast<Int32>( arrow.facing ) * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( arrow_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_bomb ( SDL_Surface* back_buffer, SDL_Surface* bomb_sheet, const Bomb& bomb,
                           Real32 camera_x, Real32 camera_y )
{

     SDL_Rect dest_rect = build_world_sdl_rect ( bomb.position.x ( ), bomb.position.y ( ),
                                                 Map::c_tile_dimension_in_meters,
                                                 Map::c_tile_dimension_in_meters );

     SDL_Rect clip_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( bomb_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_particle ( SDL_Surface* back_buffer, const Particle& particle, Uint32 color,
                              Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect = build_world_sdl_rect ( particle.position.x ( ), particle.position.y ( ), 0.0f, 0.0f );

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     dest_rect.w = 1;
     dest_rect.h = 1;

     SDL_FillRect ( back_buffer, &dest_rect, color );
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     Auto* state = get_state ( game_memory );
     Uint32 red    = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 white  = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );
     Uint32 black  = SDL_MapRGB ( back_buffer->format, 0, 0, 0 );

     // calculate camera
     state->camera.set_x ( calculate_camera_position ( back_buffer->w, state->map.width ( ),
                                                       state->player.position.x ( ), state->player.width ( ) ) );
     state->camera.set_y ( calculate_camera_position ( back_buffer->h - 16, state->map.height ( ),
                                                       state->player.position.y ( ), state->player.height ( ) ) );

     // map
     state->map_display.render ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     // interactives
     state->interactives_display.render ( back_buffer, state->interactives,
                                          state->camera.x ( ), state->camera.y ( ) );

     // enemies
     for ( Uint32 i = 0; i < state->enemies.max ( ); ++i ) {
          if ( state->enemies [ i ].is_dead ( ) ) {
               continue;
          }

          state->character_display.render_enemy ( back_buffer, state->enemies [ i ],
                                                  state->camera.x ( ), state->camera.y ( ) );
     }

     // player
     state->character_display.render_player ( back_buffer, state->player,
                                              state->camera.x ( ), state->camera.y ( ) );

     // pickups
     for ( Uint32 i = 0; i < state->pickups.max ( ); ++i ) {
          Auto& pickup = state->pickups [ i ];

          if ( pickup.is_dead ( ) ) {
               continue;
          }

          render_pickup ( back_buffer, state->pickup_sheet, pickup, state->camera.x ( ), state->camera.y ( ) );
     }

     // arrows
     for ( Uint32 i = 0; i < state->arrows.max ( ); ++i ) {
          Auto& arrow = state->arrows [ i ];

          if ( arrow.is_dead ( ) ) {
               continue;
          }

          render_arrow ( back_buffer, state->arrow_sheet, arrow, state->camera.x ( ), state->camera.y ( ) );
     }

     // bombs
     for ( Uint32 i = 0; i < state->bombs.max ( ); ++i ) {
          Auto& bomb = state->bombs [ i ];

          if ( bomb.is_dead ( ) ) {
               continue;
          }

          render_bomb ( back_buffer, state->bomb_sheet, bomb, state->camera.x ( ), state->camera.y ( ) );
     }

     // emitters
     for ( Uint32 i = 0; i < state->emitters.max ( ); ++i ) {
          Auto& emitter = state->emitters [ i ];
          if ( emitter.is_alive ( ) ) {
               for ( Uint8 i = 0; i < Emitter::c_max_particles; ++i ) {
                    Auto& particle_lifetime_watch = emitter.particle_lifetime_watches [ i ];
                    if ( !particle_lifetime_watch.expired ( ) ) {
                         render_particle ( back_buffer, emitter.particles [ i ], emitter.color,
                                           state->camera.x ( ), state->camera.y ( ) );
                    }
               }
          }
     }

     // light
     render_light ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     // ui
     SDL_Rect hud_rect { 0, 0, back_buffer->w, 16 };
     SDL_FillRect ( back_buffer, &hud_rect, black );

     // draw player health bar
     Real32 pct = static_cast<Real32>( state->player.health ) /
                  static_cast<Real32>( state->player.max_health );

     Int32 bar_len = static_cast<Int32>( 50.0f * pct );

     SDL_Rect health_bar_rect { 3, 3, bar_len, 10 };
     SDL_Rect health_bar_border_rect { 2, 2, 52, 12 };

     SDL_FillRect ( back_buffer, &health_bar_border_rect, white );
     SDL_FillRect ( back_buffer, &health_bar_rect, red );

     SDL_Rect attack_dest { ( back_buffer->w / 2 ) - ( Map::c_tile_dimension_in_pixels / 2 ), 0,
                            Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect attack_clip { state->player.attack_mode * Map::c_tile_dimension_in_pixels, 0,
                            Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( state->attack_icon_sheet, &attack_clip, back_buffer, &attack_dest );

     char buffer [ 64 ];

#ifdef LINUX
     sprintf ( buffer, "%d", state->player.key_count );
#endif

#ifdef WIN32
     sprintf_s ( buffer, "%d", state->player.key_count );
#endif
     state->text.render ( back_buffer, buffer, 235, 4 );

#ifdef LINUX
     sprintf ( buffer, "%d", state->player.bomb_count );
#endif

#ifdef WIN32
     sprintf_s ( buffer, "%d", state->player.bomb_count );
#endif
     state->text.render ( back_buffer, buffer, 210, 4 );

#ifdef LINUX
     sprintf ( buffer, "%d", state->player.arrow_count );
#endif

#ifdef WIN32
     sprintf_s ( buffer, "%d", state->player.arrow_count );
#endif
     state->text.render ( back_buffer, buffer, 185, 4 );

     SDL_Rect pickup_dest_rect { 225, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect pickup_clip_rect { Pickup::c_dimension_in_pixels, 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( state->pickup_sheet, &pickup_clip_rect, back_buffer, &pickup_dest_rect );

     SDL_Rect bomb_dest_rect { 200, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect bomb_clip_rect { Pickup::c_dimension_in_pixels * 3, 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( state->pickup_sheet, &bomb_clip_rect, back_buffer, &bomb_dest_rect );

     SDL_Rect arrow_dest_rect { 175, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect arrow_clip_rect { Pickup::c_dimension_in_pixels * 2, 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( state->pickup_sheet, &arrow_clip_rect, back_buffer, &arrow_dest_rect );
}

