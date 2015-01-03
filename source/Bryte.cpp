#include "Bryte.hpp"
#include "Utils.hpp"
#include "Globals.hpp"

using namespace bryte;

static const Real32 c_player_width       = 1.6f;
static const Real32 c_player_height      = c_player_width * 1.5f;
static const Real32 c_half_player_width  = c_player_width / 2;
static const Real32 c_half_player_height = c_player_height / 2;
static const Real32 c_player_speed       = 3.0f;

extern "C" Bool bryte_init ( GameMemory& game_memory )
{
     Globals::g_game_memory.memory = game_memory.memory;
     Globals::g_game_memory.size   = game_memory.size;

     Globals::g_memory_locations.game_state = Globals::g_game_memory.push_object<GameState> ( );

     auto* game_state = Globals::g_memory_locations.game_state;

     game_state->player_position_x = Map::c_tile_dimension * 2.0f;
     game_state->player_position_y = Map::c_tile_dimension * 2.0f;

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

extern "C" Void bryte_user_input ( SDL_Scancode scan_code, bool key_down )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     switch ( scan_code ) {
     default:
          break;
     case SDL_SCANCODE_W:
          game_state->direction_keys [ Direction::up ] = key_down;
          break;
     case SDL_SCANCODE_S:
          game_state->direction_keys [ Direction::down ] = key_down;
          break;
     case SDL_SCANCODE_A:
          game_state->direction_keys [ Direction::left ] = key_down;
          break;
     case SDL_SCANCODE_D:
          game_state->direction_keys [ Direction::right ] = key_down;
          break;
     }
}

extern "C" Void bryte_update ( Real32 time_delta )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     Real32 target_position_x = game_state->player_position_x;
     Real32 target_position_y = game_state->player_position_y;

     if ( game_state->direction_keys [ Direction::up ] ) {
          target_position_y -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::down ] ) {
          target_position_y += c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::left ] ) {
          target_position_x -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::right ] ) {
          target_position_x += c_player_speed * time_delta;
     }

     // collision
     if ( !game_state->map.is_position_solid ( target_position_x,
                                               target_position_y ) ) {
          game_state->player_position_x = target_position_x;
          game_state->player_position_y = target_position_y;
     }

     auto& player_exit = game_state->player_exit_tile_index;
     auto& map         = game_state->map;

     // check if the player has exitted
     if ( player_exit == 0 ) {
          player_exit = map.check_position_exit ( game_state->player_position_x,
                                                game_state->player_position_y );

          if ( player_exit > 0 ) {
               game_state->player_position_x = map.tile_index_to_coordinate_x ( player_exit );
               game_state->player_position_y = map.tile_index_to_coordinate_y ( player_exit );

               game_state->player_position_x *= Map::c_tile_dimension;
               game_state->player_position_y *= Map::c_tile_dimension;
          }
     } else {
          auto player_tile_index = map.position_to_tile_index ( game_state->player_position_x,
                                                                game_state->player_position_y );

          // clear the exit destination if they've left the tile
          if ( player_exit != player_tile_index ) {
               player_exit = 0;
          }
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     auto* game_state = Globals::g_memory_locations.game_state;

     Real32 camera_center_offset_x = pixels_to_meters ( back_buffer->w / 2 );
     Real32 camera_center_offset_y = pixels_to_meters ( back_buffer->h / 2 );

     game_state->camera_x = -( game_state->player_position_x - camera_center_offset_x );
     game_state->camera_y = -( game_state->player_position_y - camera_center_offset_y );

     Real32 map_width  = game_state->map.width ( ) * Map::c_tile_dimension;
     Real32 map_height = game_state->map.height ( ) * Map::c_tile_dimension;

     Real32 min_camera_x = -( map_width - pixels_to_meters ( back_buffer->w ) );
     Real32 min_camera_y = -( map_height - pixels_to_meters ( back_buffer->h ) );

     CLAMP ( game_state->camera_x, min_camera_x, 0 );
     CLAMP ( game_state->camera_y, min_camera_y, 0 );

     game_state->map.render ( back_buffer, game_state->camera_x, game_state->camera_y );

     Real32 player_screen_center_x = game_state->player_position_x + game_state->camera_x;
     Real32 player_screen_center_y = game_state->player_position_y + game_state->camera_y;

     Real32 player_screen_top_left_x = player_screen_center_x - c_half_player_width;
     Real32 player_screen_top_left_y = player_screen_center_y - c_half_player_height;

     SDL_Rect player_rect { meters_to_pixels ( player_screen_top_left_x ),
                            meters_to_pixels ( player_screen_top_left_y ),
                            meters_to_pixels ( c_player_width ), meters_to_pixels ( c_player_height ) };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

