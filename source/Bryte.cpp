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

const Real32 State::c_pickup_show_time    = 2.0f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

Void character_adjacent_tile ( const Character& character, Int32* adjacent_tile_x, Int32* adjacent_tile_y )
{
     Map::Coordinates character_center_tile = Map::vector_to_coordinates ( character.collision_center ( ) );

     switch ( character.facing ) {
          default:
               break;
          case Direction::left:
               character_center_tile.x--;
               break;
          case Direction::right:
               character_center_tile.x++;
               break;
          case Direction::up:
               character_center_tile.y++;
               break;
          case Direction::down:
               character_center_tile.y--;
               break;
     }

     *adjacent_tile_x = character_center_tile.x;
     *adjacent_tile_y = character_center_tile.y;
}

static Map::Coordinates adjacent_tile ( Map::Coordinates coords, Direction dir )
{
     switch ( dir ) {
          default:
               break;
          case Direction::left:
               coords.x--;
               break;
          case Direction::right:
               coords.x++;
               break;
          case Direction::up:
               coords.y++;
               break;
          case Direction::down:
               coords.y--;
               break;
     }

     return coords;
}

// NOTE: Windows compiler does static initialization different from gcc
//       so I cannot have the explode radius rely on map tile dimension
const Real32 Bomb::c_explode_time = 3.0f;
const Real32 Bomb::c_explode_radius = 3.2f;

Void Bomb::update ( float dt )
{
     switch ( life_state ) {
     default:
          break;
     case LifeState::spawning:
          life_state = LifeState::alive;
          break;
     case Entity::LifeState::alive:
          explode_watch.tick ( dt );

          // allow 1 frame where the bomb has expired but isn't dead
          if ( explode_watch.expired ( ) ) {
               life_state = Entity::LifeState::dying;
          }
          break;
     case Entity::LifeState::dying:
          life_state = Entity::LifeState::dead;
          break;
     }
}

Void Bomb::clear ( )
{
     explode_watch.reset ( 0.0f );
}

const Real32 DamageNumber::c_rise_height = 1.0f;
const Real32 DamageNumber::c_rise_speed = 1.0f;

Void DamageNumber::update ( float time_delta )
{
     position += Vector ( 0.0f, c_rise_speed * time_delta );

     if ( ( position.y ( ) - starting_y ) > c_rise_height ) {
          life_state = Entity::LifeState::dead;
     }
}

Void DamageNumber::clear ( )
{
     value = 0;
     starting_y = 0.0f;
     position.zero ( );
}

Bool State::initialize ( GameMemory& game_memory, Settings* settings )
{
     random.seed ( 13371 );

     player_spawn_tile_x = settings->player_spawn_tile_x;
     player_spawn_tile_y = settings->player_spawn_tile_y;

     player.clear ( );

     player.position = Map::coordinates_to_vector ( player_spawn_tile_x, player_spawn_tile_y );

     player.life_state = Entity::LifeState::alive;

     // reset keys
     for ( Int32 i = 0; i < 4; ++i ) {
          direction_keys [ i ] = false;
     }

     attack_key = false;
     item_key = false;
     switch_item_key = false;

     // clear entity managers
     pickups.clear ( );
     projectiles.clear ( );
     bombs.clear ( );
     emitters.clear ( );
     enemies.clear ( );
     damage_numbers.clear ( );

     // projectile collision for various directions
     Projectile::collision_points [ Direction::left ].set ( pixels_to_meters ( 1 ), pixels_to_meters ( 7 ) );
     Projectile::collision_points [ Direction::up ].set ( pixels_to_meters ( 7 ), pixels_to_meters ( 14 ) );
     Projectile::collision_points [ Direction::right ].set ( pixels_to_meters ( 14 ), pixels_to_meters ( 7 ) );
     Projectile::collision_points [ Direction::down ].set ( pixels_to_meters ( 7 ), pixels_to_meters ( 1 ) );

     // clear animations
     character_display.fire_animation.clear ( );
     interactives_display.animation.clear ( );
     pickup_display.animation.clear ( );
     projectile_display.animation.clear ( );
     interactives_display.clear ( );

     // clear pickup queue
     for ( Int32 i = 0; i < c_pickup_queue_size; ++i ) {
          pickup_queue [ i ] = Pickup::Type::none;
     }

     pickup_stopwatch.reset ( c_pickup_show_time );

     // load font
     if ( !text.load_surfaces ( game_memory ) ) {
          return false;
     }

     // load the region info
     if ( !region.load_info( settings->region_index ) ) {
          return false;
     }

     // load diplay surfaces
     if ( !map_display.load_surfaces ( game_memory,
                                       region.tilesheet_filepath,
                                       region.decorsheet_filepath,
                                       region.lampsheet_filepath ) ) {
          return false;
     }

     if ( !character_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     if ( !interactives_display.load_surfaces  ( game_memory ) ) {
          return false;
     }

     if ( !pickup_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     if ( !projectile_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     // load misc surfaces
     if ( !load_bitmap_with_game_memory ( bomb_sheet, game_memory, "test_bomb.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( attack_icon_sheet, game_memory, "test_attack_icon.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( player_heart_sheet, game_memory, "player_heart.bmp" ) ) {
          return false;
     }

     // load sound effects
     if ( !sound.load_effects ( ) ) {
          return false;
     }

     back_buffer_format = *map_display.tilesheet->format;

     // load map
     map.load_master_list ( region.map_list_filepath );

     if ( !map.load_from_master_list ( settings->map_index, interactives ) ) {
          return false;
     }

     spawn_map_enemies ( );
     setup_emitters_from_map_lamps ( );

#ifdef DEBUG
     enemy_think = true;
     invincible = false;
     debug_text = true;
#endif

     return true;
}

Void State::destroy ( )
{
     sound.unload_effects ( );

     map_display.unload_surfaces ( );
     character_display.unload_surfaces ( );
     interactives_display.unload_surfaces ( );
     pickup_display.unload_surfaces ( );
     projectile_display.unload_surfaces ( );

     SDL_FreeSurface ( bomb_sheet );
     SDL_FreeSurface ( attack_icon_sheet );
     SDL_FreeSurface ( player_heart_sheet );
}

Bool State::spawn_enemy ( const Vector& position, Uint8 id, Direction facing, Pickup::Type drop )
{
     Enemy* enemy = enemies.spawn ( position );

     if ( !enemy ) {
          return false;
     }

     ASSERT ( id < Enemy::Type::count );

     LOG_DEBUG ( "Spawning enemy %s at: %f, %f\n", Enemy::c_names [ id ], position.x ( ), position.y ( ) );

     enemy->init ( static_cast<Enemy::Type>( id ), position.x ( ), position.y ( ), facing, drop );

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

bool State::spawn_projectile ( Projectile::Type type, const Vector& position, Direction facing,
                               Projectile::Alliance alliance )
{
     static const Real32 c_projectile_offset_scale = Map::c_tile_dimension_in_meters * 0.5f;
     Vector offset = vector_from_direction ( facing ) * c_projectile_offset_scale;
     Auto* projectile = projectiles.spawn ( position + offset );

     if ( !projectile ) {
          return false;
     }

     projectile->type = type;
     projectile->facing = facing;
     projectile->effected_by_element = Element::none;
     projectile->alliance = alliance;

     LOG_DEBUG ( "spawning projectile at %f, %f\n", position.x ( ), position.y ( ) );

#if 0
     Auto* emitter = emitters.spawn ( position );

     if ( emitter ) {
          emitter->setup_to_track_entity ( arrow, Projectile::collision_points [ facing ],
                                           SDL_MapRGB ( &back_buffer_format, 255, 255, 255 ),
                                           0.0f, 0.0f, 0.5f, 0.5f, 0.1f, 0.1f, 1, 2 );
     }
#endif

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
                                           SDL_MapRGB ( &back_buffer_format, 255, 255, 0 ),
                                           0.785f, 2.356f, 1.0f, 1.0f, 0.5f, 0.5f, 1, 10 );
     }

     return true;
}

Bool State::spawn_damage_number ( const Vector& position, Int32 value )
{
     Auto* damage_number = damage_numbers.spawn ( position );

     if ( !damage_number ) {
          return false;
     }

     damage_number->value = -value;
     damage_number->starting_y = position.y ( );

     // NOTE: seriously?
     LOG_DEBUG ( "spawning damage number at %f, %f\n", position.x ( ), position.y ( ) );

     return true;
}

Bool State::spawn_healing_number ( const Vector& position, Int32 value )
{
     Auto* damage_number = damage_numbers.spawn ( position );

     if ( !damage_number ) {
          return false;
     }

     damage_number->value = value;
     damage_number->starting_y = position.y ( );

     // NOTE: seriously?
     LOG_DEBUG ( "spawning damage number at %f, %f\n", position.x ( ), position.y ( ) );

     return true;
}

Void State::persist_map ( )
{
     // persist map exits
     LOG_DEBUG ( "Persisting exits for map: %d\n", map.current_master_map ( ) );

     for ( Uint8 y = 0; y < interactives.height ( ); ++y ) {
          for ( Uint8 x = 0; x < interactives.width ( ); ++x ) {
               const Auto& exit = interactives.cget_from_tile ( x, y );

               if ( exit.type != Interactive::Type::exit ) {
                    continue;
               }

               map.persist_exit ( exit, x, y );
          }
     }

     // persist map enemies
     for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
          Auto& enemy = enemies [ c ];

          map.persist_enemy ( enemy, c );
     }

     map.persist_secret ( );
     map.persist_killed_all_enemies ( );
}

Void State::spawn_map_enemies ( )
{
     for ( Int32 i = 0; i < map.enemy_spawn_count ( ); ++i ) {
          Auto& enemy_spawn = map.enemy_spawn ( i );

          Vector position = Map::coordinates_to_vector ( enemy_spawn.location.x, enemy_spawn.location.y );

          spawn_enemy ( position, enemy_spawn.id, enemy_spawn.facing, enemy_spawn.drop );
     }
}

Void State::player_death ( )
{
     player.clear ( );

     player.life_state = Entity::LifeState::alive;

     player.position = Map::coordinates_to_vector ( player_spawn_tile_x, player_spawn_tile_y );

     pickups.clear ( );
     projectiles.clear ( );
     emitters.clear ( );
     enemies.clear ( );
     damage_numbers.clear ( );

     // clear persisted exits and load the first map
     map.clear_persistence ( );
     map.load_from_master_list ( 0, interactives );

     spawn_map_enemies ( );
}

Bool State::check_player_block_projectile ( Projectile& projectile )
{
     if ( player.is_blocking ( ) &&
          player.facing == opposite_direction ( projectile.facing ) ) {
          switch ( projectile.type ) {
          default:
               break;
          case Projectile::goo:
               projectile.facing = opposite_direction ( projectile.facing );
               projectile.alliance = Projectile::Alliance::good;
               break;
          case Projectile::arrow:
               projectile.track_entity.entity = &player;
               projectile.track_entity.offset = projectile.position - player.position;
               projectile.stuck_watch.reset ( Projectile::c_stuck_time );
               break;
          }

          return true;
     }

     return false;
}

Void State::enemy_death ( const Enemy& enemy )
{
     if ( enemy.drop ) {
          spawn_pickup ( enemy.position, enemy.drop );
     }

     Auto* emitter = emitters.spawn ( enemy.collision_center ( ) );

     if ( emitter ) {
          Real32 explosion_size = enemy.collision_width ( ) > enemy.collision_height ( ) ?
                                  enemy.collision_width ( ) : enemy.collision_height ( );
          explosion_size *= 2.0f;
          emitter->setup_limited_time ( enemy.collision_center ( ), 0.7f,
                                        SDL_MapRGB ( &back_buffer_format, 255, 0, 0 ),
                                        0.0f, 6.28f, 0.3f, 0.7f, 0.25f, explosion_size,
                                        Emitter::c_max_particles, 0 );
     }

     // TODO: track entity count so we don't have to do this linear check
     bool all_dead = true;

     for ( Uint8 i = 0; i < enemies.max ( ); ++i ) {
          if ( enemies [ i ].is_alive ( ) ) {
               all_dead = false;
               break;
          }
     }

     // if all the entities are dead, activate an the map's trigger
     if ( all_dead ) {
          Map::Location loc = map.activate_on_all_enemies_killed ( );
          interactives.activate ( loc.x, loc.y );
          map.killed_all_enemies ( );
     }
}

Void State::setup_emitters_from_map_lamps ( )
{
     for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
          Auto& lamp = map.lamp ( i );

          Vector position = map.location_to_vector ( lamp.location );

          Vector offset { Map::c_tile_dimension_in_meters * 0.4f,
                          Map::c_tile_dimension_in_meters * 0.7f };

          Auto* emitter = emitters.spawn ( position );

          if ( !emitter ) {
               break;
          }

          emitter->setup_immortal ( position + offset, SDL_MapRGB ( &back_buffer_format, 255, 255, 0 ),
                                    0.78f, 2.35f, 0.5f, 0.75f, 0.5f, 1.0f, 1, 10 );
     }
}

Void State::tick_character_element ( Character& character )
{
     switch ( character.effected_by_element ) {
     default:
          break;
     case Element::fire:
          character.fire_tick_count++;

          if ( character.fire_tick_count >= Character::c_fire_tick_max ) {
               // effect has expired
               character.effected_by_element = Element::none;
          } else {
               character.element_watch.reset ( Character::c_fire_tick_rate );
          }

          // push character in a random direction
          Direction dir = static_cast<Direction>( random.generate ( 0, Direction::count ) );

          damage_character ( character, c_burn_damage, dir );
          break;
     }
}

Void State::damage_character ( Character& character, Int32 amount, Direction direction )
{
     character.damage ( amount, direction );
     spawn_damage_number ( character.collision_center ( ), amount );
}

Void State::update_player ( GameMemory& game_memory, float time_delta )
{
     if ( direction_keys [ Direction::up ] ) {
          player.walk ( Direction::up );
     }

     if ( direction_keys [ Direction::down ] ) {
          player.walk ( Direction::down );
     }

     if ( direction_keys [ Direction::right ] ) {
          player.walk ( Direction::right );
     }

     if ( direction_keys [ Direction::left ] ) {
          player.walk ( Direction::left );
     }

     if ( switch_item_key ) {
          switch_item_key = false;
          Bool done = false;

          Int32 new_item_mode = static_cast< Int32 >( player.item_mode );

          while ( !done ) {
               new_item_mode++;
               new_item_mode %= Player::ItemMode::count;

               switch ( new_item_mode ) {
                    case Player::ItemMode::shield:
                         done = true;
                         break;
                    case Player::ItemMode::arrow:
                         if ( player.arrow_count ) {
                              done = true;
                         }
                         break;
                    case Player::ItemMode::bomb:
                         if ( player.bomb_count ) {
                              done = true;
                         }
                         break;
               }
          }

          player.item_mode = static_cast<Player::ItemMode>( new_item_mode );
     }

     if ( attack_key ) {
          attack_key = false;

          if ( player.attack ( ) ) {
               sound.play_effect ( Sound::Effect::player_sword_attack );
          }
     }

     player.update ( time_delta, map, interactives );

     tick_character_element ( player );

     if ( player.is_dead ( ) ) {
          player_death ( );
     }

     player.item_cooldown.tick ( time_delta );

     if ( item_key ) {
          switch ( player.item_mode ) {
          default:
               ASSERT ( 0 );
               break;
          case Player::ItemMode::shield:
               player.block ( );
               break;
          case Player::ItemMode::arrow:
               if ( player.item_cooldown.expired ( ) && player.arrow_count > 0 ) {
                    spawn_projectile ( Projectile::Type::arrow, player.position, player.facing,
                                       Projectile::Alliance::good );
                    player.arrow_count--;
                    player.item_cooldown.reset ( Player::c_item_cooldown );
               }
               break;
          case Player::ItemMode::bomb:
               if ( player.item_cooldown.expired ( ) && player.bomb_count > 0 ) {
                    spawn_bomb ( player.position );
                    player.bomb_count--;
                    sound.play_effect ( Sound::Effect::place_bomb );
                    player.item_cooldown.reset ( Player::c_item_cooldown );
               }
               break;
          }
     }

     Map::Coordinates player_center_tile = Map::vector_to_coordinates ( player.collision_center ( ) );

     if ( map.coordinates_valid ( player_center_tile ) ) {
          Auto& interactive = interactives.get_from_tile ( player_center_tile.x, player_center_tile.y );
          Direction border_side = player_on_border ( );

          // check if player is trying to exit the map
          if ( interactive.type == Interactive::Type::exit &&
               interactive.interactive_exit.state == Exit::State::open &&
               interactive.interactive_exit.direction == opposite_direction ( player.facing ) ) {
               Int32 map_index = interactive.interactive_exit.map_index;
               Int32 region_index = interactive.interactive_exit.region_index;
               Vector new_position = Map::coordinates_to_vector ( interactive.interactive_exit.exit_index_x,
                                                                  interactive.interactive_exit.exit_index_y );

               new_position += Vector ( Map::c_tile_dimension_in_meters * 0.5f,
                                        Map::c_tile_dimension_in_meters * 0.5f );

               if ( region_index != region.current_index ) {
                    change_region ( game_memory, region_index );
               }

               change_map ( map_index );

               player.set_collision_center ( new_position.x ( ), new_position.y ( ) );

               LOG_DEBUG ( "Setting player to %f, %f\n",
                           player.position.x ( ),
                           player.position.y ( ) );

               // no need to finish this update
               return;
          } else if ( border_side != Direction::count &&
                      border_side == player.facing ) {
               Auto& border_exit = map.get_border_exit ( border_side );
               Int32 map_index = border_exit.map_index;

               Auto player_offset = player.collision_center ( ) -
                                    Map::location_to_vector ( border_exit.bottom_left );
               Auto new_player_pos = Map::location_to_vector ( border_exit.map_bottom_left ) + player_offset;

               change_map ( map_index );

               player.set_collision_center ( new_player_pos.x ( ), new_player_pos.y ( ) );

               LOG_DEBUG ( "Changing to map %d, setting player to %d, %d\n",
                           map_index,
                           player.position.x ( ),
                           player.position.y ( ) );

               return;
          }
     }

     if ( player.is_pushing ( ) ) {
          Map::Coordinates push_location { 0, 0 };

          character_adjacent_tile ( player, &push_location.x, &push_location.y );

          if ( push_location.x >= 0 && push_location.x < interactives.width ( ) &&
               push_location.y >= 0 && push_location.y < interactives.height ( ) ) {

               Bool enemy_on_tile = false;
               Auto dest = adjacent_tile ( push_location, player.facing );

               for ( Uint8 i = 0; i < enemies.max ( ); ++i ) {
                    if ( enemies [ i ].is_dead ( ) ) {
                         continue;
                    }

                    Auto coords = Map::vector_to_coordinates ( enemies [ i ].collision_center ( ) );

                    if ( coords.x == dest.x && coords.y == dest.y ) {
                         enemy_on_tile = true;
                         break;
                    }
               }

               if ( !enemy_on_tile ) {
                    Bool pushed = interactives.push ( push_location.x, push_location.y, player.facing, map );

                    if ( pushed ) {
                         sound.play_effect ( Sound::Effect::activate_interactive );
                    }
               }
          }
     } else {
          // check if player wants to activate any interactives
          if ( activate_key ) {
               activate_key = false;

               Int32 player_activate_tile_x = 0;
               Int32 player_activate_tile_y = 0;

               character_adjacent_tile ( player, &player_activate_tile_x, &player_activate_tile_y );

               if ( map.coordinates_valid ( Map::Coordinates { player_activate_tile_x,
                                                               player_activate_tile_y } ) ) {
                    Auto& interactive = interactives.get_from_tile ( player_activate_tile_x,
                                                                     player_activate_tile_y );

                    if ( interactive.type == Interactive::Type::exit ) {
                         if ( interactive.interactive_exit.state == Exit::State::locked &&
                              player.key_count > 0 ) {
                              LOG_DEBUG ( "Unlock Door: %d, %d\n", player_activate_tile_x, player_activate_tile_y );
                              interactives.activate ( player_activate_tile_x, player_activate_tile_y );
                              player.key_count--;
                              sound.play_effect ( Sound::Effect::activate_interactive );
                         }
                    } else if ( interactive.type == Interactive::Type::torch ||
                                interactive.type == Interactive::Type::pushable_torch ||
                                interactive.underneath.type == UnderneathInteractive::Type::popup_block ) {
                         // pass
                    } else {
                         LOG_DEBUG ( "Activate: %d, %d\n", player_activate_tile_x, player_activate_tile_y );

                         Bool success = interactives.activate ( player_activate_tile_x, player_activate_tile_y );

                         if ( success ) {
                              sound.play_effect ( Sound::Effect::activate_interactive );
                         }
                    }

                    Auto& secret_location = map.secret ( ).location;

                    if ( player_activate_tile_x == secret_location.x &&
                         player_activate_tile_y == secret_location.y ) {
                         sound.play_effect ( Sound::Effect::activate_interactive );
                         map.find_secret ( );
                         LOG_DEBUG ( "Found secret!\n" );
                    }
               }
          }
     }

#ifdef DEBUG
     if ( invincible ) {
          player.health = player.max_health;
     }
#endif
}

Void State::update_enemies ( float time_delta )
{
     Vector player_center = player.collision_center ( );

     for ( Uint32 i = 0; i < enemies.max ( ); ++i ) {
          Auto& enemy = enemies [ i ];

          if ( enemy.is_dead ( ) ) {
               continue;
          }

          Vector enemy_center = enemy.collision_center ( );

#ifdef DEBUG
          if ( enemy_think ) {
               enemy.think ( enemies.entities, enemies.max ( ), player, random, time_delta );
          }
#else
          enemy.think ( enemies.entities, enemies.max ( ), player, random, time_delta );
#endif

          enemy.update ( time_delta, map, interactives );

          if ( enemy.type == Enemy::Type::fairy &&
               enemy.fairy_state.heal_timer.expired ( ) ) {
               heal_enemies_in_range_of_fairy ( enemy.position );
          }

          // check if the enemy has died after updating
          if ( enemy.is_dead ( ) ) {
               enemy_death ( enemy );
               continue;
          }

          // spawn a projectile if the goo is shooting
          if ( enemy.type == Enemy::Type::goo &&
               enemy.goo_state.state == Enemy::GooState::State::shooting ) {
               spawn_projectile ( Projectile::Type::goo, enemy.position, enemy.facing,
                                  Projectile::Alliance::evil );
          }

          tick_character_element ( enemy );

          // check collision between player and enemy
          if ( !player.is_blinking ( ) && player.is_alive ( ) &&
               !enemy.is_blinking ( ) && player.collides_with ( enemy ) ) {
               Direction damage_dir = direction_between ( enemy_center, player_center, random );

               // check if player blocked the attack
               if ( player.is_blocking ( ) &&
                    damage_dir == opposite_direction ( player.facing ) ) {
                    damage_character ( enemy, c_block_damage, opposite_direction ( damage_dir ) ) ;
                    spawn_pickup ( enemy.position, enemy.drop );
                    enemy.drop = Pickup::Type::none;
               } else {
                    damage_character ( player, c_enemy_damage, damage_dir );
                    sound.play_effect ( Sound::Effect::player_damaged );
                    enemy.hit_player = true;
               }

#ifdef DEBUG
               if ( invincible ) {
                    player.health = player.max_health;
               }
#endif

               player.effect_with_element ( enemy.effected_by_element );
          }

          // check if player's attack hits enemy
          if ( player.is_attacking ( ) && !enemy.is_blinking ( ) &&
               player.attack_collides_with ( enemy ) ) {
               Direction damage_dir = direction_between ( player_center, enemy_center, random );

               damage_character ( enemy, c_attack_damage, damage_dir );

               sound.play_effect ( Sound::Effect::player_damaged );
          }
     }
}

Void State::update_interactives ( float time_delta )
{
     Int32 count = map.width ( ) * map.height ( );

     for ( Int32 i = 0; i < count; ++i ) {
          Auto& interactive = interactives.m_interactives [ i ];

          if ( interactive.underneath.type == UnderneathInteractive::Type::ice &&
               interactive.underneath.underneath_ice.force_dir != Direction::count ) {
               Int32 tile_x = map.tile_index_to_coordinate_x ( i );
               Int32 tile_y = map.tile_index_to_coordinate_y ( i );
               interactives.push ( tile_x, tile_y, interactive.underneath.underneath_ice.force_dir, map );
          }

          if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector &&
               interactive.underneath.underneath_ice_detector.detected &&
               interactive.underneath.underneath_ice_detector.force_dir != Direction::count ) {
               Int32 tile_x = map.tile_index_to_coordinate_x ( i );
               Int32 tile_y = map.tile_index_to_coordinate_y ( i );
               interactives.push ( tile_x, tile_y,
                                   interactive.underneath.underneath_ice_detector.force_dir, map );
          }

          if ( interactive.underneath.type == UnderneathInteractive::Type::moving_walkway &&
               interactive.underneath.underneath_ice.force_dir != Direction::count ) {
               Int32 tile_x = map.tile_index_to_coordinate_x ( i );
               Int32 tile_y = map.tile_index_to_coordinate_y ( i );
               interactives.push ( tile_x, tile_y, interactive.underneath.underneath_moving_walkway.facing, map );
          }

          switch ( interactive.type ) {
          default:
          case Interactive::Type::none:
          case Interactive::Type::bombable_block:
               break;
          case Interactive::Type::torch:
               if ( interactive.interactive_torch.element ) {
                    Auto& element = interactive.interactive_pushable_torch.torch.element;
                    Int32 tile_x = map.tile_index_to_coordinate_x ( i );
                    Int32 tile_y = map.tile_index_to_coordinate_y ( i );
                    interactives.spread_ice ( tile_x, tile_y, map,
                                              element == Element::ice ? false : true );
               }
               break;
          case Interactive::Type::lever:
               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::pushable_block:
               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::pushable_torch:
               if ( interactive.interactive_pushable_torch.torch.element ) {
                    Auto& element = interactive.interactive_pushable_torch.torch.element;
                    Int32 tile_x = map.tile_index_to_coordinate_x ( i );
                    Int32 tile_y = map.tile_index_to_coordinate_y ( i );

                    interactives.spread_ice ( tile_x, tile_y, map,
                                              element == Element::ice ? false : true );
               }

               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::exit:
               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::turret:
               if ( interactive.interactive_turret.wants_to_shoot ) {
                    Int32 tile_x = map.tile_index_to_coordinate_x ( i );
                    Int32 tile_y = map.tile_index_to_coordinate_y ( i );
                    Vector interactive_pos = Map::coordinates_to_vector ( tile_x, tile_y );

                    spawn_projectile ( Projectile::Type::arrow,
                                       interactive_pos,
                                       interactive.interactive_turret.facing,
                                       Projectile::Alliance::neutral );

               }

               interactive.update ( time_delta, interactives );
               break;
          }
     }
}

Void State::update_projectiles ( float time_delta )
{
     for ( Uint32 i = 0; i < projectiles.max ( ); ++i ) {
          Auto& projectile = projectiles [ i ];

          if ( projectile.is_dead ( ) ) {
               continue;
          }

          projectile.update ( time_delta, map, interactives );

          if ( !projectile.stuck_watch.expired ( ) ) {
               continue;
          }

          Vector projectile_collision_point = projectile.position + projectile.collision_points [ projectile.facing ];

          switch ( projectile.alliance ) {
          default:
               break;
          case Projectile::Alliance::good:
               for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
                    Auto& enemy = enemies [ c ];

                    if ( enemy.is_dead ( ) ) {
                         continue;
                    }

                    if ( point_inside_rect ( projectile_collision_point.x ( ),
                                             projectile_collision_point.y ( ),
                                             enemy.collision_x ( ), enemy.collision_y ( ),
                                             enemy.collision_x ( ) + enemy.collision_width ( ),
                                             enemy.collision_y ( ) + enemy.collision_height ( ) ) ) {
                         Int32 damage = projectile.hit_character ( enemy );
                         spawn_damage_number ( projectile_collision_point, damage );
                         break;
                    }
               }
               break;
         case Projectile::Alliance::evil:
               if ( point_inside_rect ( projectile_collision_point.x ( ),
                                        projectile_collision_point.y ( ),
                                        player.collision_x ( ), player.collision_y ( ),
                                        player.collision_x ( ) + player.collision_width ( ),
                                        player.collision_y ( ) + player.collision_height ( ) ) ) {
                    if ( !check_player_block_projectile ( projectile ) ) {
                         Int32 damage = projectile.hit_character ( player );
                         spawn_damage_number ( projectile_collision_point, damage );
                         sound.play_effect ( Sound::Effect::player_damaged );
                    }
               }
               break;
         case Projectile::Alliance::neutral:
               for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
                    Auto& enemy = enemies [ c ];

                    if ( enemy.is_dead ( ) ) {
                         continue;
                    }

                    if ( point_inside_rect ( projectile_collision_point.x ( ),
                                             projectile_collision_point.y ( ),
                                             enemy.collision_x ( ), enemy.collision_y ( ),
                                             enemy.collision_x ( ) + enemy.collision_width ( ),
                                             enemy.collision_y ( ) + enemy.collision_height ( ) ) ) {
                         Int32 damage = projectile.hit_character ( enemy );
                         spawn_damage_number ( projectile_collision_point, damage );
                         break;
                    }
               }

               if ( point_inside_rect ( projectile_collision_point.x ( ),
                                        projectile_collision_point.y ( ),
                                        player.collision_x ( ), player.collision_y ( ),
                                        player.collision_x ( ) + player.collision_width ( ),
                                        player.collision_y ( ) + player.collision_height ( ) ) ) {
                    if ( !check_player_block_projectile ( projectile ) ) {
                         Int32 damage = projectile.hit_character ( player );
                         spawn_damage_number ( projectile_collision_point, damage );
                         sound.play_effect ( Sound::Effect::player_damaged );
                    }
               }

               break;
         }
     }
}

Void State::update_bombs ( float time_delta )
{
     for ( Uint32 i = 0; i < bombs.max ( ); ++i ) {
          Auto& bomb = bombs [ i ];

          if ( bomb.is_dead ( ) ) {
               continue;
          }

          bomb.update ( time_delta );

          Vector bomb_center { bomb.position.x ( ) + Map::c_tile_dimension_in_meters * 0.5f,
                               bomb.position.y ( ) + Map::c_tile_dimension_in_meters * 0.5f };
          Int32 bomb_tile_x = meters_to_pixels ( bomb_center.x ( ) ) / Map::c_tile_dimension_in_pixels;
          Int32 bomb_tile_y = meters_to_pixels ( bomb_center.y ( ) ) / Map::c_tile_dimension_in_pixels;

          Auto& current_interactive = interactives.get_from_tile ( bomb_tile_x, bomb_tile_y );

          if ( current_interactive.underneath.type == UnderneathInteractive::Type::moving_walkway ) {
               static const Real32 c_bomb_moving_walkway_speed = 0.05f;
               Direction moving = current_interactive.underneath.underneath_moving_walkway.facing;
               bomb.position += vector_from_direction ( moving ) * c_bomb_moving_walkway_speed;
          }

          if ( bomb.life_state == Entity::LifeState::dying ) {
               // damage nearby enemies
               for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
                    Auto& enemy = enemies [ c ];

                    if ( enemy.is_dead ( ) ) {
                         continue;
                    }

                    Vector enemy_center = enemy.collision_center ( );

                    if ( enemy_center.distance_to ( bomb.position ) < Bomb::c_explode_radius ) {
                         damage_character ( enemy, c_bomb_damage, direction_between ( bomb.position,
                                                                                      enemy_center,
                                                                                      random ) );
                    }
               }

               // determine nearby objects
               Vector bomb_center { bomb.position.x ( ) + Map::c_tile_dimension_in_meters * 0.5f,
                                    bomb.position.y ( ) + Map::c_tile_dimension_in_meters * 0.5f };
               Int32 tile_radius = meters_to_pixels ( Bomb::c_explode_radius ) / Map::c_tile_dimension_in_pixels;
               Int32 tile_min_x = meters_to_pixels ( bomb_center.x ( ) ) / Map::c_tile_dimension_in_pixels;
               Int32 tile_min_y = meters_to_pixels ( bomb_center.y ( ) ) / Map::c_tile_dimension_in_pixels;

               tile_min_x -= tile_radius;
               tile_min_y -= tile_radius;

               Int32 tile_max_x = tile_min_x + tile_radius * 2;
               Int32 tile_max_y = tile_min_y + tile_radius * 2;

               CLAMP ( tile_min_x, 0, interactives.width ( ) - 1 );
               CLAMP ( tile_min_y, 0, interactives.height ( ) - 1 );
               CLAMP ( tile_max_x, 0, interactives.width ( ) - 1 );
               CLAMP ( tile_max_y, 0, interactives.height ( ) - 1 );

               // loop over nearby objects
               for ( Int32 y = tile_min_y; y <= tile_max_y; ++y ) {
                    for ( Int32 x = tile_min_x; x <= tile_max_x; ++x ) {
                         interactives.explode ( x, y );
                    }
               }

               // create quick emitter
               Auto* emitter = emitters.spawn ( bomb.position );

               if ( emitter ) {
                    Vector offset { Map::c_tile_dimension_in_meters * 0.5f,
                                    Map::c_tile_dimension_in_meters * 0.5f };
                    // TODO: Make fire color
                    emitter->setup_limited_time ( bomb.position + offset, 0.5f,
                                                  SDL_MapRGB ( &back_buffer_format, 200, 200, 200 ),
                                                  0.0f, 6.28f, 0.5f, 0.5f, 6.0f, 6.0f,
                                                  Emitter::c_max_particles, 0 );
               }

               sound.play_effect ( Sound::Effect::bomb_exploded );
          }
     }
}

Void State::update_pickups ( float time_delta )
{
     for ( Uint32 i = 0; i < pickups.max ( ); ++i ) {
          Pickup& pickup = pickups [ i ];

          if ( pickup.is_dead ( ) ) {
               continue;
          }

          if ( rect_collides_with_rect ( player.collision_x ( ), player.collision_y ( ),
                                         player.collision_width ( ), player.collision_height ( ),
                                         pickup.position.x ( ), pickup.position.y ( ),
                                         Pickup::c_dimension_in_meters,
                                         Pickup::c_dimension_in_meters ) ||
               ( player.is_attacking ( ) &&
                 rect_collides_with_rect ( player.attack_x ( ), player.attack_y ( ),
                                           player.attack_width ( ), player.attack_height ( ),
                                           pickup.position.x ( ), pickup.position.y ( ),
                                           Pickup::c_dimension_in_meters,
                                           Pickup::c_dimension_in_meters ) ) ) {
               LOG_DEBUG ( "Player got pickup %s\n", Pickup::c_names [ pickup.type ] );

               switch ( pickup.type ) {
               default:
                    break;
               case Pickup::Type::health:
                    player.health += 2;

                    if ( player.health > player.max_health ) {
                         player.health = player.max_health;
                    }
                    break;
               case Pickup::Type::key:
                    player.key_count++;
                    break;
               case Pickup::Type::arrow:
                    player.arrow_count += 5;
                    break;
               case Pickup::Type::bomb:
                    player.bomb_count++;
                    break;
               }

               enqueue_pickup ( pickup.type );

               pickup.type = Pickup::Type::none;
               pickup.life_state = Entity::LifeState::dead;

               sound.play_effect ( Sound::Effect::player_pickup );
          }
     }

     if ( pickup_queue [ 0 ] ) {
          pickup_stopwatch.tick ( time_delta );

          if ( pickup_stopwatch.expired ( ) ) {
               dequeue_pickup ( );
          }
     }
}

Void State::update_emitters ( float time_delta )
{
     for ( Uint32 i = 0; i < emitters.max ( ); ++i ) {
          Auto& emitter = emitters [ i ];

          if ( emitter.is_dead ( ) ) {
               continue;
          }

          emitter.update ( time_delta, random );
     }
}

Void State::update_damage_numbers ( float time_delta )
{
     for ( Uint32 i = 0; i < damage_numbers.max ( ); ++i ) {
          Auto& damage_number = damage_numbers [ i ];

          if ( damage_number.is_dead ( ) ) {
               continue;
          }

          damage_number.update ( time_delta );
     }
}


Void State::update_light ( )
{
     map.reset_light ( );

     interactives.contribute_light ( map );

     // projectiles on fire contribute light
     for ( Uint32 i = 0; i < projectiles.max ( ); ++i ) {
          Auto& arrow = projectiles [ i ];

          if ( arrow.is_dead ( ) ) {
               continue;
          }

          if ( arrow.effected_by_element == Element::fire ) {
               map.illuminate ( meters_to_pixels ( arrow.position.x ( ) ),
                                meters_to_pixels ( arrow.position.y ( ) ),
                                192 );
          }
     }

     // give interactives the light values on their respective tiles
     for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
               interactives.light ( x, y, map.get_coordinate_light ( x, y ) );
          }
     }
}

Void State::enqueue_pickup ( Pickup::Type type )
{
     for( Int32 i = 0; i < c_pickup_queue_size; ++i ) {
          if ( pickup_queue [ i ] == Pickup::Type::none ) {
               pickup_queue [ i ] = type;
               break;
          }
     }
}

Void State::dequeue_pickup ( )
{
     Int32 last_index = c_pickup_queue_size - 1;

     for( Int32 i = 0; i < last_index; ++i ) {
          pickup_queue [ i ] = pickup_queue [ i + 1 ];
     }

     pickup_stopwatch.reset ( c_pickup_show_time );
}

Void State::heal_enemies_in_range_of_fairy ( const Vector& position )
{
     for ( Uint32 i = 0; i < enemies.max ( ); ++i ) {
          Auto& enemy = enemies [ i ];

          if ( enemy.is_dead ( ) || enemy.type == Enemy::Type::fairy ) {
               continue;
          }

          if ( enemy.position.distance_to ( position ) < Enemy::FairyState::c_heal_radius ) {
               enemy.heal ( 1 );
               spawn_healing_number ( enemy.collision_center ( ), 1 );
          }
     }
}

Void State::change_map ( Int32 map_index )
{
     LOG_DEBUG ( "Changing map to %d\n", map_index );

     persist_map ( );
     map.load_from_master_list ( map_index, interactives );

     pickups.clear ( );
     projectiles.clear ( );
     enemies.clear ( );
     emitters.clear ( );

     spawn_map_enemies ( );

     setup_emitters_from_map_lamps ( );
}

Direction State::player_on_border ( )
{
     Auto player_tile = Map::vector_to_coordinates ( player.collision_center ( ) );

     Auto border = map.get_border_exit ( Direction::left );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.x == border.bottom_left.x ) {
               LOG_DEBUG ( "Player on left border\n" );
               return Direction::left;
          }
     }

     border = map.get_border_exit ( Direction::right );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.x == border.bottom_left.x ) {
               LOG_DEBUG ( "Player on right border\n" );
               return Direction::right;
          }
     }

     border = map.get_border_exit ( Direction::up );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.y == border.bottom_left.y ) {
               LOG_DEBUG ( "Player on up border\n" );
               return Direction::up;
          }
     }

     border = map.get_border_exit ( Direction::down );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.y == border.bottom_left.y ) {
               LOG_DEBUG ( "Player on down border\n" );
               return Direction::down;
          }
     }

     return Direction::count;
}

Bool State::change_region ( GameMemory& game_memory, Int32 region_index )
{
     LOG_INFO ( "Changing to region %d\n", region_index );

     // load new region info
     if ( !region.load_info ( region_index ) ) {
          return false;
     }

     // unload and re-load surfaces
     map_display.unload_surfaces ( );

     if ( !map_display.load_surfaces ( game_memory,
                                       region.tilesheet_filepath,
                                       region.decorsheet_filepath,
                                       region.lampsheet_filepath ) ) {
          return false;
     }

     // load a new map master list and clear persistence
     map.load_master_list ( region.map_list_filepath );
     map.clear_persistence ( );

     return true;
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

     // handle keyboard
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
               state->switch_item_key = key_change.down;
               break;
          case SDL_SCANCODE_SPACE:
               state->attack_key = key_change.down;
               break;
          case SDL_SCANCODE_LCTRL:
               state->item_key = key_change.down;
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
          case SDL_SCANCODE_P:
               if ( key_change.down ) {
                    state->invincible = !state->invincible;
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
          case SDL_SCANCODE_T:
               if ( key_change.down ) {
                    state->debug_text = !state->debug_text;
               }
               break;
#endif
          }
     }

     // handle controller
     for ( Uint32 i = 0; i < game_input.controller_button_change_count; ++i ) {
          const GameInput::ButtonChange& btn_change = game_input.controller_button_changes [ i ];

          switch ( btn_change.button ) {
               default:
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    state->direction_keys [ Direction::left ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    state->direction_keys [ Direction::up ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    state->direction_keys [ Direction::right ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    state->direction_keys [ Direction::down ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_A:
                    state->attack_key = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_B:
                    state->item_key = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_X:
                    state->activate_key = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_Y:
                    state->switch_item_key = btn_change.down;
                    break;
          }
     }

#ifdef DEBUG
     for ( Uint32 i = 0; i < game_input.mouse_button_change_count; ++i ) {
          Auto change = game_input.mouse_button_changes [ i ];

          switch ( change.button ) {
          default:
               break;
          case SDL_BUTTON_LEFT:
          {
               if ( change.down ) {
                    Int32 mouse_screen_x = game_input.mouse_position_x - meters_to_pixels ( state->camera.x ( ) );
                    Int32 mouse_screen_y = game_input.mouse_position_y - meters_to_pixels ( state->camera.y ( ) );

                    state->player.set_collision_center ( pixels_to_meters ( mouse_screen_x ),
                                                         pixels_to_meters ( mouse_screen_y ) );
               }
          } break;
          }
     }
#endif
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     Auto* state = get_state ( game_memory );

     state->update_player ( game_memory, time_delta );
     state->update_enemies ( time_delta );
     state->update_interactives ( time_delta );
     state->update_projectiles ( time_delta );
     state->update_bombs ( time_delta );
     state->update_pickups ( time_delta );
     state->update_emitters ( time_delta );
     state->update_damage_numbers ( time_delta );
     state->update_light ( );
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

static void render_damage_number ( Text& text, SDL_Surface* back_buffer, const DamageNumber& damage_number,
                                   Real32 camera_x, Real32 camera_y )
{
     char buffer [ 64 ];
     Int32 value = abs ( damage_number.value );
     const char* positive_format = "+%d";
     const char* negative_format = "%d";
     const char* format = damage_number.value > 0 ? positive_format : negative_format;

#if LINUX
     sprintf ( buffer, format, value );
#endif

#if WIN32
     sprintf_s ( buffer, format, value );
#endif

     SDL_Rect dest_rect = build_world_sdl_rect ( damage_number.position.x ( ), damage_number.position.y ( ),
                                                 pixels_to_meters ( text.character_width ),
                                                 pixels_to_meters ( text.character_height ) );

     dest_rect.x -= ( text.character_width / 2 );
     dest_rect.y -= ( text.character_height / 2 );

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     text.render ( back_buffer, buffer, dest_rect.x, dest_rect.y);
}

Void render_shown_pickup ( SDL_Surface* back_buffer, SDL_Surface* pickup_sheet,
                           Character& player, Pickup::Type pickup_type,
                           Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect = build_world_sdl_rect ( player.position.x ( ) + Pickup::c_dimension_in_meters * 0.4f,
                                                 player.position.y ( ) + player.height ( ) + pixels_to_meters ( 1 ),
                                                 Pickup::c_dimension_in_meters,
                                                 Pickup::c_dimension_in_meters );

     SDL_Rect clip_rect { 0, ( static_cast<Int32>( pickup_type ) - 1 ) * Pickup::c_dimension_in_pixels,
                          Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( pickup_sheet, &clip_rect, back_buffer, &dest_rect );
}

Void render_icon ( SDL_Surface* back_buffer, SDL_Surface* icon_sheet, Int32 frame, Int32 x, Int32 y )
{
     Uint32 white  = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );

     SDL_Rect attack_dest { x, y, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect attack_clip { frame * Map::c_tile_dimension_in_pixels, 0,
                            Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( icon_sheet, &attack_clip, back_buffer, &attack_dest );

     SDL_Rect attack_outline { attack_dest.x - 1, attack_dest.y - 1,
                               Map::c_tile_dimension_in_pixels + 2,
                               Map::c_tile_dimension_in_pixels + 2 };

     render_rect_outline ( back_buffer, attack_outline, white );
}

Void render_hearts ( SDL_Surface* back_buffer, SDL_Surface* heart_sheet, Int32 health, Int32 max_health,
                     Int32 x, Int32 y )
{
     Int32 heart_count = max_health / 2;
     Int32 heart_state = 0;

     SDL_Rect clip_rect { 0, 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect dest_rect { x, y, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     for ( Int32 i = 0; i < heart_count; ++i ) {
          heart_state += 2;

          if ( heart_state == health + 1 ) {
               clip_rect.x = Pickup::c_dimension_in_pixels;
          } else if ( heart_state > health ) {
               clip_rect.x = 2 * Pickup::c_dimension_in_pixels;
          }

          SDL_BlitSurface ( heart_sheet, &clip_rect, back_buffer, &dest_rect );

          dest_rect.x += Pickup::c_dimension_in_pixels + 2;
          clip_rect.x = 0;
     }
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     Auto* state = get_state ( game_memory );
     Uint32 black  = SDL_MapRGB ( back_buffer->format, 0, 0, 0 );

     state->back_buffer_format = *back_buffer->format;

     // calculate camera
     state->camera.set_x ( calculate_camera_position ( back_buffer->w, state->map.width ( ),
                                                       state->player.position.x ( ), state->player.width ( ) ) );
     state->camera.set_y ( calculate_camera_position ( back_buffer->h - Map::c_tile_dimension_in_pixels,
                                                       state->map.height ( ),
                                                       state->player.position.y ( ), state->player.height ( ) ) );

     // map
     state->map_display.render ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ),
                                 state->map.found_secret ( ) );

     // interactives
     state->interactives_display.tick ( );
     state->interactives_display.render ( back_buffer, state->interactives, state->map,
                                          state->camera.x ( ), state->camera.y ( ), state->map.found_secret ( ) );

     state->character_display.tick ( );

     // enemies in 2 passes, non-flying and flying
     for ( Uint32 i = 0; i < state->enemies.max ( ); ++i ) {
          Auto& enemy = state->enemies [ i ];
          if ( enemy.is_dead ( ) || enemy.flies ) {
               continue;
          }

          state->character_display.render_enemy ( back_buffer, enemy,
                                                  state->camera.x ( ), state->camera.y ( ) );
     }

     // player
     state->character_display.render_player ( back_buffer, state->player,
                                              state->camera.x ( ), state->camera.y ( ) );

     for ( Uint32 i = 0; i < state->enemies.max ( ); ++i ) {
          Auto& enemy = state->enemies [ i ];
          if ( enemy.is_dead ( ) || !enemy.flies ) {
               continue;
          }

          state->character_display.render_enemy ( back_buffer, enemy,
                                                  state->camera.x ( ), state->camera.y ( ) );
     }

     // pickups
     state->pickup_display.tick ( );
     for ( Uint32 i = 0; i < state->pickups.max ( ); ++i ) {
          Auto& pickup = state->pickups [ i ];

          if ( pickup.is_dead ( ) ) {
               continue;
          }

          state->pickup_display.render ( back_buffer, pickup, state->camera.x ( ), state->camera.y ( ) );
     }

     // projectiles
     state->projectile_display.tick ( );

     for ( Uint32 i = 0; i < state->projectiles.max ( ); ++i ) {
          Auto& projectile = state->projectiles [ i ];

          if ( projectile.is_dead ( ) ) {
               continue;
          }

          state->projectile_display.render ( back_buffer, projectile,
                                             state->camera.x ( ), state->camera.y ( ) );
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

     // pickup queue
     if ( state->pickup_queue [ 0 ] ) {
          render_shown_pickup ( back_buffer, state->pickup_display.pickup_sheet, state->player,
                                state->pickup_queue [ 0 ], state->camera.x ( ), state->camera.y ( ) );
     }

     // light
     render_light ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     // damage numbers
     for ( Uint32 i = 0; i < state->damage_numbers.max ( ); ++i ) {
          Auto& damage_number = state->damage_numbers [ i ];

          if ( damage_number.is_dead ( ) ) {
               continue;
          }

          render_damage_number ( state->text, back_buffer, damage_number,
                                 state->camera.x ( ), state->camera.y ( ) );
     }

     // ui
     SDL_Rect hud_rect { 0, 0, back_buffer->w, Map::c_tile_dimension_in_pixels };
     SDL_FillRect ( back_buffer, &hud_rect, black );

     render_hearts ( back_buffer, state->player_heart_sheet, state->player.health, state->player.max_health,
                     2, 2 );

     render_icon ( back_buffer, state->attack_icon_sheet, 0,
                   ( back_buffer->w / 2 ) - Map::c_tile_dimension_in_pixels, 1 );

     render_icon ( back_buffer, state->attack_icon_sheet, state->player.item_mode + 1,
                   ( back_buffer->w / 2 ) + 3, 1 );

     char buffer [ 64 ];

#ifdef WIN32
     sprintf_s ( buffer, "%d", state->player.key_count );
#else
     sprintf ( buffer, "%d", state->player.key_count );
#endif

     state->text.render ( back_buffer, buffer, 235, 4 );

#ifdef WIN32
     sprintf_s ( buffer, "%d", state->player.bomb_count );
#else
     sprintf ( buffer, "%d", state->player.bomb_count );
#endif

     state->text.render ( back_buffer, buffer, 210, 4 );

#ifdef WIN32
     sprintf_s ( buffer, "%d", state->player.arrow_count );
#else
     sprintf ( buffer, "%d", state->player.arrow_count );
#endif

     state->text.render ( back_buffer, buffer, 185, 4 );

     SDL_Rect pickup_dest_rect { 225, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect pickup_clip_rect { 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( state->pickup_display.pickup_sheet, &pickup_clip_rect, back_buffer, &pickup_dest_rect );

     SDL_Rect bomb_dest_rect { 200, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect bomb_clip_rect { 0, Pickup::c_dimension_in_pixels * 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( state->pickup_display.pickup_sheet, &bomb_clip_rect, back_buffer, &bomb_dest_rect );

     SDL_Rect arrow_dest_rect { 175, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect arrow_clip_rect { 0, Pickup::c_dimension_in_pixels * 2, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( state->pickup_display.pickup_sheet, &arrow_clip_rect, back_buffer, &arrow_dest_rect );

#ifdef DEBUG

     if ( state->debug_text ) {
          Auto player_coord = Map::vector_to_coordinates ( state->player.position );

          // TODO: I wish I could just use snprintf so it is 'safe' on all platforms, but it doesn't seem to exist on windows?
#ifdef WIN32
          sprintf_s (
#else
          sprintf (
#endif
                    buffer, "P %.2f %.2f  T %d %d  M %d  AI %s  INV %s",
                    state->player.position.x ( ), state->player.position.y ( ),
                    player_coord.x, player_coord.y,
                    state->map.current_master_map ( ),
                    state->enemy_think ? "ON" : "OFF",
                    state->invincible ? "ON" : "OFF" );

          state->text.render ( back_buffer, buffer, 0, 230 );
     }
#endif
}

