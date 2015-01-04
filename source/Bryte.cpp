#include "Bryte.hpp"
#include "Utils.hpp"
#include "Globals.hpp"

using namespace bryte;

static const Real32 c_player_speed = 3.0f;

Bool Character::collides_with ( Real32 new_x, Real32 new_y, const Character& character )
{
     Real32 character_right = character.position_x + character.width;
     Real32 character_top   = character.position_y + character.collision_height;

     if ( point_inside_rect ( new_x, new_y,
                              character.position_x, character.position_y,
                              character_right, character_top ) ||
          point_inside_rect ( new_x + width, new_y,
                              character.position_x, character.position_y,
                              character_right, character_top ) ||
          point_inside_rect ( new_x, new_y + collision_height,
                              character.position_x, character.position_y,
                              character_right, character_top ) ||
          point_inside_rect ( new_x + width, new_y + collision_height,
                              character.position_x, character.position_y,
                              character_right, character_top ) ) {
          return true;
     }

     Real32 right = new_x + width;
     Real32 top = new_y + collision_height;

     if ( point_inside_rect ( character.position_x, character.position_y,
                              new_x, new_y,
                              right, top ) ||
          point_inside_rect ( character.position_x + character.width, character.position_y,
                              new_x, new_y,
                              right, top ) ||
          point_inside_rect ( character.position_x, character.position_y + character.collision_height,
                              new_x, new_y,
                              right, top ) ||
          point_inside_rect ( character.position_x + character.width,
                              character.position_y + character.collision_height,
                              new_x, new_y,
                              right, top ) ) {
          return true;
     }

     return false;
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

     game_state->enemy.position_x       = Map::c_tile_dimension * 5.0f;
     game_state->enemy.position_y       = Map::c_tile_dimension * 5.0f;
     game_state->enemy.width            = 1.0f;
     game_state->enemy.height           = game_state->enemy.width * 1.5f;
     game_state->enemy.collision_height = game_state->enemy.width;

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
          }
     }
}

extern "C" Void bryte_update ( Real32 time_delta )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     Real32 target_position_x = game_state->player.position_x;
     Real32 target_position_y = game_state->player.position_y;

     if ( game_state->direction_keys [ Direction::up ] ) {
          target_position_y += c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::down ] ) {
          target_position_y -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::left ] ) {
          target_position_x -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::right ] ) {
          target_position_x += c_player_speed * time_delta;
     }

     bool collided  = false;

     // collision
     if ( game_state->map.is_position_solid ( target_position_x,
                                              target_position_y ) ||
          game_state->map.is_position_solid ( target_position_x + game_state->player.width,
                                              target_position_y ) ||
          game_state->map.is_position_solid ( target_position_x,
                                              target_position_y + game_state->player.collision_height ) ||
          game_state->map.is_position_solid ( target_position_x + game_state->player.width,
                                              target_position_y + game_state->player.collision_height ) ) {
          collided = true;
     }

     // check collision between player and enemy
     if ( game_state->player.collides_with ( target_position_x, target_position_y, game_state->enemy ) ) {
          collided = true;
     }

     if ( !collided ) {
          game_state->player.position_x = target_position_x;
          game_state->player.position_y = target_position_y;
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

}

Void render_character ( SDL_Surface* back_buffer, const Character& character,
                        Real32 camera_x, Real32 camera_y, Uint32 color )
{
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

     Uint32 red  = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 blue = SDL_MapRGB ( back_buffer->format, 0, 0, 255 );

     render_character ( back_buffer, game_state->player, game_state->camera_x, game_state->camera_y, red );
     render_character ( back_buffer, game_state->enemy, game_state->camera_x, game_state->camera_y, blue );
}

