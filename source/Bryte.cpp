#include "Bryte.hpp"
#include "Utils.hpp"
#include "Globals.hpp"

using namespace bryte;

static const Real32 c_player_speed            = 3.0f;
static const Real32 c_character_damage_speed  = 12.0f;
static const Real32 c_character_damage_force  = 0.8f;
static const Real32 c_character_attack_width  = 0.6f;
static const Real32 c_character_attack_height = 1.2f;
static const Real32 c_character_attack_time   = 0.5f;

static Bool rect_collides_with_rect ( Real32 a_left, Real32 a_bottom, Real32 a_width, Real32 a_height,
                                      Real32 b_left, Real32 b_bottom, Real32 b_width, Real32 b_height)
{
     Real32 b_right = b_left + b_width;
     Real32 b_top = b_bottom + b_height;

     // test A inside B
     if ( point_inside_rect ( a_left, a_bottom,
                              b_left, b_bottom, b_right, b_top ) ||
          point_inside_rect ( a_left + a_width, a_bottom,
                              b_left, b_bottom, b_right, b_top ) ||
          point_inside_rect ( a_left, a_bottom + a_height,
                              b_left, b_bottom, b_right, b_top ) ||
          point_inside_rect ( a_left + a_width, a_bottom + a_height,
                              b_left, b_bottom, b_right, b_top ) ) {
          return true;
     }

     Real32 a_right = a_left + a_width;
     Real32 a_top = a_bottom + a_height;

     // test B inside A
     if ( point_inside_rect ( b_left, b_bottom,
                              a_left, a_bottom, a_right, a_top ) ||
          point_inside_rect ( b_left + b_width, b_bottom,
                              a_left, a_bottom, a_right, a_top ) ||
          point_inside_rect ( b_left, b_bottom + b_height,
                              a_left, a_bottom, a_right, a_top ) ||
          point_inside_rect ( b_left + b_width, b_bottom + b_height,
                              a_left, a_bottom, a_right, a_top ) ) {
          return true;
     }

     return false;
}

Bool Character::collides_with ( Real32 new_x, Real32 new_y, const Character& character )
{
     return rect_collides_with_rect ( new_x, new_y, width, height,
                                      character.position_x, character.position_y,
                                      character.width, character.height );
}

Void Character::attack ( )
{
     attack_time = c_character_attack_time;
}

Bool Character::attack_collides_with ( const Character& character )
{
     return rect_collides_with_rect ( attack_x, attack_y, c_character_attack_width, c_character_attack_height,
                                      character.position_x, character.position_y,
                                      character.width, character.height );
}

Void Character::damage ( Int32 amount, Direction push )
{
     health -= amount;

     switch ( push ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          damage_move_x = -c_character_damage_force;
          damage_move_y = 0.0f;
          break;
     case Direction::right:
          damage_move_x = c_character_damage_force;
          damage_move_y = 0.0f;
          break;
     case Direction::up:
          damage_move_x = 0.0f;
          damage_move_y = c_character_damage_force;
          break;
     case Direction::down:
          damage_move_x = 0.0f;
          damage_move_y = -c_character_damage_force;
          break;
     }
}

Void Character::update ( Real32 time_delta )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     Real32 target_position_x = position_x + velocity_x * time_delta;
     Real32 target_position_y = position_y + velocity_y * time_delta;

     if ( damage_move_y > 0.0f ) {
          target_position_y += c_character_damage_speed * time_delta;
          damage_move_y     -= c_character_damage_speed * time_delta;
     }

     bool collided = false;

     // collision
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

     velocity_x = 0.0f;
     velocity_y = 0.0f;
}

extern "C" Bool bryte_init ( GameMemory& game_memory )
{
     Globals::g_game_memory.memory = game_memory.memory;
     Globals::g_game_memory.size   = game_memory.size;

     Globals::g_memory_locations.game_state = Globals::g_game_memory.push_object<GameState> ( );

     auto* game_state = Globals::g_memory_locations.game_state;

     game_state->player.position_x       = Map::c_tile_dimension * 2.0f;
     game_state->player.position_y       = Map::c_tile_dimension * 2.0f;
     game_state->player.width            = 1.6f;
     game_state->player.height           = game_state->player.width * 1.5f;
     game_state->player.collision_height = game_state->player.width;
     game_state->player.health           = 100;

     game_state->enemy.position_x       = Map::c_tile_dimension * 5.0f;
     game_state->enemy.position_y       = Map::c_tile_dimension * 5.0f;
     game_state->enemy.width            = 1.0f;
     game_state->enemy.height           = game_state->enemy.width * 1.5f;
     game_state->enemy.collision_height = game_state->enemy.width;
     game_state->enemy.health           = 10;

     game_state->map.build ( );

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_reload_memory ( GameMemory& game_memory )
{
     Globals::g_game_memory.memory = game_memory.memory;
     Globals::g_game_memory.size   = game_memory.size;

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
          game_state->player.facing = Direction::down;
     }

     if ( game_state->direction_keys [ Direction::right ] ) {
          game_state->player.velocity_x = c_player_speed;
          game_state->player.facing = Direction::right;
     }

     if ( game_state->direction_keys [ Direction::left ] ) {
          game_state->player.velocity_x = -c_player_speed;
          game_state->player.facing = Direction::left;
     }

     if ( game_state->attack_key ) {
          game_state->player.attack ( );
     }

     game_state->player.update ( time_delta );
     game_state->enemy.update ( time_delta );

     // check collision between player and enemy
#if 0
     if ( game_state->enemy.health > 0 &&
          game_state->player.collides_with ( target_position_x, target_position_y, game_state->enemy ) ) {
          collided = true;

          game_state->player.damage ( 1, Direction::up );
     }
#endif

     auto& player_exit = game_state->player_exit_tile_index;
     auto& map         = game_state->map;

     // check if the player has exitted the area
     if ( player_exit == 0 ) {
          player_exit = map.check_position_exit ( game_state->player.position_x,
                                                game_state->player.position_y );

          if ( player_exit > 0 ) {
               game_state->player.position_x = map.tile_index_to_coordinate_x ( player_exit );
               game_state->player.position_y = map.tile_index_to_coordinate_y ( player_exit );

               game_state->player.position_x *= Map::c_tile_dimension;
               game_state->player.position_y *= Map::c_tile_dimension;
          }
     } else {
          auto player_tile_index = map.position_to_tile_index ( game_state->player.position_x,
                                                                game_state->player.position_y );

          // clear the exit destination if they've left the tile
          if ( player_exit != player_tile_index ) {
               player_exit = 0;
          }
     }

     switch ( game_state->player.facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          game_state->player.attack_x = game_state->player.position_x - 0.2f;
          game_state->player.attack_y = game_state->player.position_y + game_state->player.height / 2.0f;
          break;
     case Direction::right:
          game_state->player.attack_x = game_state->player.position_x + game_state->player.width + 0.2f;
          game_state->player.attack_y = game_state->player.position_y + game_state->player.height / 2.0f;
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

     if ( game_state->player.attack_time > 0.0f ) {
          game_state->player.attack_time -= time_delta;

          if ( game_state->player.attack_collides_with ( game_state->enemy ) ) {
               game_state->enemy.damage ( 1, Direction::up );
          }
     }
}

Void render_character ( SDL_Surface* back_buffer, const Character& character,
                        Real32 camera_x, Real32 camera_y, Uint32 color )
{
     // do not draw if dead
     if ( character.health <= 0 ) {
          return;
     }

     Real32 character_on_camera_x = character.position_x + camera_x;
     Real32 character_on_camera_y = character.position_y + camera_y;

     SDL_Rect character_rect { meters_to_pixels ( character_on_camera_x ),
                               meters_to_pixels ( character_on_camera_y ),
                               meters_to_pixels ( character.width ),
                               meters_to_pixels ( character.height ) };


     convert_to_sdl_origin_for_surface ( character_rect, back_buffer );

     SDL_FillRect ( back_buffer, &character_rect, color );
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     Real32 camera_center_offset_x = pixels_to_meters ( back_buffer->w / 2 );
     Real32 camera_center_offset_y = pixels_to_meters ( back_buffer->h / 2 );

     Real32 half_player_width = game_state->player.width / 2.0f;
     Real32 half_player_height = game_state->player.height / 2.0f;

     game_state->camera_x = -( game_state->player.position_x + half_player_width - camera_center_offset_x );
     game_state->camera_y = -( game_state->player.position_y + half_player_height - camera_center_offset_y );

     Real32 map_width  = game_state->map.width ( ) * Map::c_tile_dimension;
     Real32 map_height = game_state->map.height ( ) * Map::c_tile_dimension;

     Real32 min_camera_x = -( map_width - pixels_to_meters ( back_buffer->w ) );
     Real32 min_camera_y = -( map_height - pixels_to_meters ( back_buffer->h ) );

     CLAMP ( game_state->camera_x, min_camera_x, 0 );
     CLAMP ( game_state->camera_y, min_camera_y, 0 );

     game_state->map.render ( back_buffer, game_state->camera_x, game_state->camera_y );

     Uint32 red   = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 blue  = SDL_MapRGB ( back_buffer->format, 0, 0, 255 );
     Uint32 green = SDL_MapRGB ( back_buffer->format, 0, 255, 0 );
     Uint32 white = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );

     render_character ( back_buffer, game_state->player, game_state->camera_x, game_state->camera_y, red );
     render_character ( back_buffer, game_state->enemy, game_state->camera_x, game_state->camera_y, blue );

     if ( game_state->player.attack_time > 0.0f ) {
          SDL_Rect attack_rect { meters_to_pixels ( game_state->player.attack_x + game_state->camera_x ),
                                 meters_to_pixels ( game_state->player.attack_y + game_state->camera_y ),
                                 meters_to_pixels ( c_character_attack_width ),
                                 meters_to_pixels ( c_character_attack_height ) };

          convert_to_sdl_origin_for_surface ( attack_rect, back_buffer );

          SDL_FillRect ( back_buffer, &attack_rect, green );
     }

     // health bar
     Real32 pct = static_cast<Real32>( game_state->player.health ) /
                  static_cast<Real32>( game_state->player.max_health );

     Int32 bar_len = static_cast<Int32>( 50.0f * pct );

     SDL_Rect health_bar_rect { 15, 15, bar_len, 10 };
     SDL_Rect health_bar_border_rect { 14, 14, 52, 12 };

     SDL_FillRect ( back_buffer, &health_bar_border_rect, white );
     SDL_FillRect ( back_buffer, &health_bar_rect, red );
}

