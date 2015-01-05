#include "Bryte.hpp"
#include "Utils.hpp"
#include "Globals.hpp"

using namespace bryte;

static const Real32 c_player_speed            = 3.0f;
static const Real32 c_character_damage_speed  = 10.0f;
static const Real32 c_character_damage_time   = 0.15f;
static const Real32 c_character_blink_time    = 1.5f;
static const Real32 c_character_attack_width  = 0.6f;
static const Real32 c_character_attack_height = 1.2f;
static const Real32 c_character_attack_time   = 0.35f;
static const Real32 c_character_cooldown_time = 0.25f;
static const Real32 c_lever_width             = 0.5f;
static const Real32 c_lever_height            = 0.5f;
static const Real32 c_lever_activate_cooldown = 0.75f;
static const Char8* c_test_tilesheet_path     = "castle_tilesheet.bmp";

Bool Character::collides_with ( const Character& character )
{
     return rect_collides_with_rect ( position_x, position_y, width, collision_height,
                                      character.position_x, character.position_y,
                                      character.width, character.collision_height );
}

Void Character::attack ( )
{
     if ( attack_time > 0.0f || cooldown_time > 0.0f ) {
          return;
     }

     attack_time = c_character_attack_time;
     state       = State::attacking;
}

Bool Character::attack_collides_with ( const Character& character )
{
     // swap width/height based on direction we are facing
     switch ( facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::up:
     case Direction::down:
          return rect_collides_with_rect ( attack_x, attack_y,
                                           c_character_attack_width, c_character_attack_height,
                                           character.position_x, character.position_y,
                                           character.width, character.height );
     case Direction::left:
     case Direction::right:
          return rect_collides_with_rect ( attack_x, attack_y,
                                           c_character_attack_height, c_character_attack_width,
                                           character.position_x, character.position_y,
                                           character.width, character.height );
     }

     return false;
}

Void Character::damage ( Int32 amount, Direction push )
{
     health      -= amount;
     damage_push = push;
     damage_time = c_character_damage_time;
     blink_time  = c_character_blink_time;

     if ( health > 0 ) {
          state = State::blinking;
     } else {
          state = State::dead;
     }
}

Void Character::update ( Real32 time_delta )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     Real32 target_position_x = position_x + velocity_x * time_delta;
     Real32 target_position_y = position_y + velocity_y * time_delta;

     if ( damage_time > 0.0f ) {
          damage_time -= time_delta;

          switch ( damage_push ) {
          default:
               ASSERT ( 0 );
               break;
          case Direction::left:
               target_position_x -= c_character_damage_speed * time_delta;
               break;
          case Direction::right:
               target_position_x += c_character_damage_speed * time_delta;
               break;
          case Direction::up:
               target_position_y += c_character_damage_speed * time_delta;
               break;
          case Direction::down:
               target_position_y -= c_character_damage_speed * time_delta;
               break;
          }

          if ( damage_time < 0.0f ) {
               damage_time = 0.0f;
          }
     }

     if ( blink_time > 0.0f ) {
          blink_time -= time_delta;

          if ( blink_time < 0.0f ) {
               blink_time = 0.0f;

               if ( state != State::dead ) {
                    state = State::alive;
               }
          }
     }

     bool collided = false;

     // collision with tile map
     if ( game_state->map.is_position_solid ( target_position_x,
                                              target_position_y ) ||
          game_state->map.is_position_solid ( target_position_x + width,
                                              target_position_y ) ||
          game_state->map.is_position_solid ( target_position_x,
                                              target_position_y + collision_height ) ||
          game_state->map.is_position_solid ( target_position_x + width,
                                              target_position_y + collision_height ) ) {
          collided = true;
     }

     if ( !collided ) {
          position_x = target_position_x;
          position_y = target_position_y;
     }

     if ( cooldown_time > 0.0f ) {
          cooldown_time -= time_delta;
     }

     if ( attack_time > 0.0f ) {
          attack_time -= time_delta;

          if ( attack_time < 0.0f ) {
               attack_time   = 0.0f;
               cooldown_time = c_character_cooldown_time;

               if ( state == State::attacking ) {
                    state = State::alive;
               }
          }
     }

     switch ( game_state->player.facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          game_state->player.attack_x = game_state->player.position_x - c_character_attack_height;
          game_state->player.attack_y = game_state->player.position_y + game_state->player.height * 0.5f;
          break;
     case Direction::right:
          game_state->player.attack_x = game_state->player.position_x + game_state->player.width;
          game_state->player.attack_y = game_state->player.position_y + game_state->player.height * 0.5f;
          break;
     case Direction::up:
          game_state->player.attack_x = game_state->player.position_x + game_state->player.width * 0.33f;
          game_state->player.attack_y = game_state->player.position_y + game_state->player.height;
          break;
     case Direction::down:
          game_state->player.attack_x = game_state->player.position_x + game_state->player.width * 0.33f;
          game_state->player.attack_y = game_state->player.position_y - game_state->player.height * 0.5f;
          break;
     }

     velocity_x = 0.0f;
     velocity_y = 0.0f;
}

Void Random::seed ( Uint32 value )
{
     i_f = value;
     i_s = value;
}

Uint32 Random::generate ( Uint32 min, Uint32 max )
{
     if ( min == max ) {
          return min;
     }

     // simple RNG by George Marsaglia
     i_f = 36969 * ( i_f & 65535 ) + ( i_f >> 16 );
     i_s = 18000 * ( i_s & 65535 ) + ( i_s >> 16 );

     return ( ( ( i_f << 16 ) + i_s ) % ( max - min ) ) + min;
}

Bool GameState::initialize ( )
{
     random.seed ( 41491 );

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

     player.damage_push = Direction::left;
     player.damage_time = 0.0f;
     player.blink_time  = 0.0f;

     player.attack_x = 0.0f;
     player.attack_y = 0.0f;

     player.attack_time   = 0.0f;
     player.cooldown_time = 0.0f;

     // ensure all enemies start dead
     for ( Uint32 i = 0; i < c_max_enemies; ++i ) {
          enemies [ i ].state = Character::State::dead;
     }

     lever.position_x      = Map::c_tile_dimension_in_meters * 4.0f;
     lever.position_y      = Map::c_tile_dimension_in_meters * 7.5f - c_lever_width * 0.5f;
     lever.activate_tile_x = 3;
     lever.activate_tile_y = 8;
     lever.activate_time   = 0.0f;

     LOG_INFO ( "Loading tilesheet '%s'\n", c_test_tilesheet_path );

     tilesheet = SDL_LoadBMP ( c_test_tilesheet_path );

     if ( !tilesheet ) {
          LOG_ERROR ( "SDL_LoadBMP(): %s\n", c_test_tilesheet_path,
                      SDL_GetError ( ) );
          return false;
     }

     return true;
}

Bool GameState::spawn_enemy ( Real32 x, Real32 y )
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

     enemy->state  = Character::State::alive;
     enemy->facing = Direction::left;

     enemy->health     = 10;
     enemy->max_health = 10;

     enemy->position_x = x;
     enemy->position_y = y;

     enemy->velocity_x = 0.0f;
     enemy->velocity_y = 0.0f;

     enemy->width            = 1.0f;
     enemy->height           = enemy->width * 1.5f;
     enemy->collision_height = enemy->width;

     enemy->damage_push = Direction::left;
     enemy->damage_time = 0.0f;
     enemy->blink_time  = 0.0f;

     enemy->attack_x    = 0.0f;
     enemy->attack_y    = 0.0f;

     enemy->attack_time   = 0.0f;
     enemy->cooldown_time = 0.0f;

     enemy_count++;

     return true;
}

// assuming A attacks B
static Direction determine_damage_direction ( const Character& a, const Character& b )
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
          return valid_dirs [ Globals::g_memory_locations.game_state->random.generate ( 0, 2 ) ];
     }

     // the above cases should catch all
     ASSERT ( 0 );
     return Direction::left;
}

static Void render_character ( SDL_Surface* back_buffer, const Character& character,
                               Real32 camera_x, Real32 camera_y, Uint32 color )
{
     static const Int32 blink_length  = 3;
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

     SDL_Rect character_rect { meters_to_pixels ( character.position_x ),
                               meters_to_pixels ( character.position_y ),
                               meters_to_pixels ( character.width ),
                               meters_to_pixels ( character.height ) };

     world_to_sdl ( character_rect, back_buffer, camera_x, camera_y );

     SDL_FillRect ( back_buffer, &character_rect, color );
}

static Void render_map ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                         Real32 camera_x, Real32 camera_y )
{
     ASSERT ( map.m_current_room );

     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {

               SDL_Rect tile_rect { 0, 0,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
               SDL_Rect clip_rect { 0, 0,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

               auto tile_value = map.get_coordinate_value ( x, y );

               clip_rect.x = tile_value * Map::c_tile_dimension_in_pixels;

               tile_rect.x = x * Map::c_tile_dimension_in_pixels;
               tile_rect.y = y * Map::c_tile_dimension_in_pixels;

               world_to_sdl ( tile_rect, back_buffer, camera_x, camera_y );

               SDL_BlitSurface ( tilesheet, &clip_rect, back_buffer, &tile_rect );
          }
     }

     Uint32 exit_color = SDL_MapRGB ( back_buffer->format, 0, 170, 0 );

     for ( Uint8 d = 0; d < map.m_current_room->exit_count; ++d ) {
          auto& exit = map.m_current_room->exits [ d ];

          SDL_Rect exit_rect { 0, 0,
                               Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

          exit_rect.x = exit.location_x * Map::c_tile_dimension_in_pixels;
          exit_rect.y = exit.location_y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( exit_rect, back_buffer, camera_x, camera_y );

          SDL_FillRect ( back_buffer, &exit_rect, exit_color );
     }
}

static Void setup_game_state_from_memory ( GameMemory& game_memory )
{
     ASSERT ( game_memory.memory );
     ASSERT ( game_memory.size );

     Globals::g_game_memory.memory = game_memory.memory;
     Globals::g_game_memory.size   = game_memory.size;

     Globals::g_memory_locations.game_state = Globals::g_game_memory.push_object<GameState> ( );
}

extern "C" Bool bryte_init ( GameMemory& game_memory )
{
     setup_game_state_from_memory ( game_memory );

     auto* game_state = Globals::g_memory_locations.game_state;

     if ( !game_state->initialize ( ) ) {
          return false;
     }

     game_state->map.build ( );

     game_state->map.set_coordinate_value ( 1, 7, 4 );
     game_state->map.set_coordinate_value ( 2, 7, 4 );
     game_state->map.set_coordinate_value ( 3, 7, 4 );
     game_state->map.set_coordinate_value ( 1, 9, 6 );
     game_state->map.set_coordinate_value ( 2, 9, 6 );
     game_state->map.set_coordinate_value ( 3, 9, 6 );
     game_state->map.set_coordinate_value ( 3, 8, 5 );

     for ( Uint32 i = 0; i < 2; ++i ) {
          Int32 max_tries = 10;
          Int32 random_tile_x = 0;
          Int32 random_tile_y = 0;

          while ( max_tries > 0 ) {
               random_tile_x = game_state->random.generate ( 0, 16 );
               random_tile_y = game_state->random.generate ( 0, 16 );

               if ( !game_state->map.is_position_solid ( random_tile_x, random_tile_y ) ) {
                    break;
               }

               max_tries--;
          }

          game_state->spawn_enemy ( Map::c_tile_dimension_in_meters * static_cast<Real32>( random_tile_x ),
                                    Map::c_tile_dimension_in_meters * static_cast<Real32>( random_tile_y ) );
     }

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_reload_memory ( GameMemory& game_memory )
{
     setup_game_state_from_memory ( game_memory );

     auto* game_state = Globals::g_memory_locations.game_state;

     game_state->map.build ( );
}

extern "C" Void bryte_user_input ( const GameInput& game_input )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_W:
               game_state->direction_keys [ Direction::up ]    = key_change.down;
               break;
          case SDL_SCANCODE_S:
               game_state->direction_keys [ Direction::down ]  = key_change.down;
               break;
          case SDL_SCANCODE_A:
               game_state->direction_keys [ Direction::left ]  = key_change.down;
               break;
          case SDL_SCANCODE_D:
               game_state->direction_keys [ Direction::right ] = key_change.down;
               break;
          case SDL_SCANCODE_C:
               game_state->attack_key = key_change.down;
               break;
          case SDL_SCANCODE_E:
               game_state->activate_key = key_change.down;
               break;
          case SDL_SCANCODE_8:
               game_state->spawn_enemy ( game_state->player.position_x - game_state->player.width,
                                         game_state->player.position_y );
               break;
          }
     }
}

extern "C" Void bryte_update ( Real32 time_delta )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     if ( game_state->direction_keys [ Direction::up ] ) {
          game_state->player.velocity_y = c_player_speed;
          game_state->player.facing     = Direction::up;
     }

     if ( game_state->direction_keys [ Direction::down ] ) {
          game_state->player.velocity_y = -c_player_speed;
          game_state->player.facing     = Direction::down;
     }

     if ( game_state->direction_keys [ Direction::right ] ) {
          game_state->player.velocity_x = c_player_speed;
          game_state->player.facing     = Direction::right;
     }

     if ( game_state->direction_keys [ Direction::left ] ) {
          game_state->player.velocity_x = -c_player_speed;
          game_state->player.facing     = Direction::left;
     }

     if ( game_state->attack_key ) {
          game_state->player.attack ( );
     }

     if ( game_state->activate_key ) {
          if ( game_state->lever.activate_time <= 0.0f ) {
               auto& player = game_state->player;
               auto& lever  = game_state->lever;

               if ( rect_collides_with_rect ( player.position_x, player.position_y,
                                              player.width, player.height,
                                              lever.position_x, lever.position_y,
                                              c_lever_width, c_lever_height ) ) {
                    auto tile_value = game_state->map.get_coordinate_value ( lever.activate_tile_x,
                                                                             lever.activate_tile_y );

                    Uint8 id = tile_value ? 0 : 5;

                    game_state->map.set_coordinate_value ( lever.activate_tile_x, lever.activate_tile_y, id );

                    game_state->lever.activate_time = c_lever_activate_cooldown;
               }
          }
     }

     game_state->player.update ( time_delta );

     for ( Uint32 i = 0; i < GameState::c_max_enemies; ++i ) {
          auto& enemy = game_state->enemies [ i ];

          if ( enemy.state == Character::State::dead ) {
               continue;
          }

          enemy.update ( time_delta );

          // check collision between player and enemy
          if ( game_state->player.state != Character::State::blinking &&
               game_state->player.collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( enemy, game_state->player );
               game_state->player.damage ( 1, damage_dir );
          }

          // attacking enemy
          if ( game_state->player.attack_time > 0.0f &&
               enemy.state != Character::State::blinking &&
               game_state->player.attack_collides_with ( enemy ) ) {
               Direction damage_dir = determine_damage_direction ( game_state->player, enemy );
               enemy.damage ( 1, damage_dir );
          }
     }

     auto& player_exit = game_state->player_exit_tile_index;
     auto& map         = game_state->map;

     // check if the player has exitted the area
     if ( player_exit == 0 ) {
          player_exit = map.check_position_exit ( game_state->player.position_x,
                                                  game_state->player.position_y );

          if ( player_exit > 0 ) {
               game_state->player.position_x = map.tile_index_to_coordinate_x ( player_exit );
               game_state->player.position_y = map.tile_index_to_coordinate_y ( player_exit );

               game_state->player.position_x *= Map::c_tile_dimension_in_meters;
               game_state->player.position_y *= Map::c_tile_dimension_in_meters;
          }
     } else {
          auto player_tile_index = map.position_to_tile_index ( game_state->player.position_x,
                                                                game_state->player.position_y );

          // clear the exit destination if they've left the tile
          if ( player_exit != player_tile_index ) {
               player_exit = 0;
          }
     }

     if ( game_state->lever.activate_time > 0.0f ) {
          game_state->lever.activate_time -= time_delta;
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     // calculate camera position based on player
     Real32 camera_center_offset_x = pixels_to_meters ( back_buffer->w / 2 );
     Real32 camera_center_offset_y = pixels_to_meters ( back_buffer->h / 2 );

     Real32 half_player_width = game_state->player.width * 0.5f;
     Real32 half_player_height = game_state->player.height * 0.5f;

     game_state->camera_x = -( game_state->player.position_x + half_player_width - camera_center_offset_x );
     game_state->camera_y = -( game_state->player.position_y + half_player_height - camera_center_offset_y );

     Real32 map_width  = game_state->map.width ( ) * Map::c_tile_dimension_in_meters;
     Real32 map_height = game_state->map.height ( ) * Map::c_tile_dimension_in_meters;

     Real32 min_camera_x = -( map_width - pixels_to_meters ( back_buffer->w ) );
     Real32 min_camera_y = -( map_height - pixels_to_meters ( back_buffer->h ) );

     CLAMP ( game_state->camera_x, min_camera_x, 0 );
     CLAMP ( game_state->camera_y, min_camera_y, 0 );

     // draw map
     render_map ( back_buffer, game_state->tilesheet, game_state->map,
                  game_state->camera_x, game_state->camera_y );

     Uint32 red     = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 blue    = SDL_MapRGB ( back_buffer->format, 0, 0, 255 );
     Uint32 green   = SDL_MapRGB ( back_buffer->format, 0, 255, 0 );
     Uint32 white   = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );
     Uint32 magenta = SDL_MapRGB ( back_buffer->format, 255, 0, 255 );

     // draw lever
     SDL_Rect lever_rect { meters_to_pixels ( game_state->lever.position_x ),
                           meters_to_pixels ( game_state->lever.position_y ),
                           meters_to_pixels ( c_lever_width ),
                           meters_to_pixels ( c_lever_height ) };

     world_to_sdl ( lever_rect, back_buffer, game_state->camera_x, game_state->camera_y );

     SDL_FillRect ( back_buffer, &lever_rect, magenta );

     // draw enemies
     for ( Uint32 i = 0; i < game_state->enemy_count; ++i ) {
          render_character ( back_buffer, game_state->enemies [ i ],
                             game_state->camera_x, game_state->camera_y, blue );
     }

     // draw player
     render_character ( back_buffer, game_state->player, game_state->camera_x, game_state->camera_y, red );

     // draw player attack
     if ( game_state->player.state == Character::State::attacking ) {
          SDL_Rect attack_rect { meters_to_pixels ( game_state->player.attack_x ),
                                 meters_to_pixels ( game_state->player.attack_y ),
                                 meters_to_pixels ( c_character_attack_width ),
                                 meters_to_pixels ( c_character_attack_height ) };

          // swap width and height for facing left and right
          if ( game_state->player.facing == Direction::left ||
               game_state->player.facing == Direction::right ) {
               attack_rect.w = meters_to_pixels ( c_character_attack_height );
               attack_rect.h = meters_to_pixels ( c_character_attack_width );
          }

          world_to_sdl ( attack_rect, back_buffer, game_state->camera_x, game_state->camera_y );

          SDL_FillRect ( back_buffer, &attack_rect, green );
     }

     // draw player health bar
     Real32 pct = static_cast<Real32>( game_state->player.health ) /
                  static_cast<Real32>( game_state->player.max_health );

     Int32 bar_len = static_cast<Int32>( 50.0f * pct );

     SDL_Rect health_bar_rect { 15, 15, bar_len, 10 };
     SDL_Rect health_bar_border_rect { 14, 14, 52, 12 };

     SDL_FillRect ( back_buffer, &health_bar_border_rect, white );
     SDL_FillRect ( back_buffer, &health_bar_rect, red );
}

