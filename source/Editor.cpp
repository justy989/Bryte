#include "Editor.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "MapDisplay.hpp"

using namespace editor;

const Real32 State::c_camera_speed = 20.0f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

extern "C" Bool game_init ( GameMemory& game_memory, void* settings )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State);

     state->settings = reinterpret_cast<Settings*>( settings );

     memory_locations->state = state;

     state->room.initialize ( state->settings->map_width, state->settings->map_height,
                              GAME_PUSH_MEMORY_ARRAY ( game_memory, bryte::Map::Tile,
                                                       state->settings->map_width * state->settings->map_height ) );

     state->map.set_current_room ( &state->room );

     FileContents bitmap_contents = load_entire_file ( state->settings->map_tilesheet_filename, &game_memory );
     state->tilesheet = load_bitmap ( &bitmap_contents );
     if ( !state->tilesheet ) {
          return false;
     }

     state->current_tile = 1;

     state->camera_x = 0.0f;
     state->camera_y = 0.0f;

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{
     State* state = get_state ( game_memory );

     SDL_FreeSurface ( state->tilesheet );
}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     State* state = get_state ( game_memory );

     for ( Uint32 i = 0; i < game_input.mouse_button_change_count; ++i ) {
          auto change = game_input.mouse_button_changes [ i ];
          if ( change.down && ( SDL_BUTTON(SDL_BUTTON_LEFT) & change.button ) ) {
               Int32 tx = game_input.mouse_position_x - meters_to_pixels ( state->camera_x );
               Int32 ty = game_input.mouse_position_y - meters_to_pixels ( state->camera_y );

               tx /= bryte::Map::c_tile_dimension_in_pixels;
               ty /= bryte::Map::c_tile_dimension_in_pixels;

               if ( tx >= 0 && tx < state->map.width ( ) && ty >= 0 && ty < state->map.height ( ) ) {
                    state->map.set_coordinate_value ( tx, ty, state->current_tile );
               }
          }
     }

     state->mouse_x = game_input.mouse_position_x;
     state->mouse_y = game_input.mouse_position_y;

     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_Q:
               if ( key_change.down ) {
                    state->current_tile--;
               }
               break;
          case SDL_SCANCODE_E:
               if ( key_change.down ) {
                    state->current_tile++;
               }
               break;
          case SDL_SCANCODE_W:
               state->camera_direction_keys [ 0 ] = key_change.down;
               break;
          case SDL_SCANCODE_S:
               state->camera_direction_keys [ 1 ] = key_change.down;
               break;
          case SDL_SCANCODE_D:
               state->camera_direction_keys [ 2 ] = key_change.down;
               break;
          case SDL_SCANCODE_A:
               state->camera_direction_keys [ 3 ] = key_change.down;
               break;
          case SDL_SCANCODE_O:
               if ( key_change.down ) {
                    state->map.m_current_room->save ( state->settings->map_save_filename );
               }
               break;
          case SDL_SCANCODE_I:
               if ( key_change.down ) {
                    state->map.m_current_room->load ( state->settings->map_save_filename );
               }
               break;
          }
     }
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     State* state = get_state ( game_memory );

     if ( state->camera_direction_keys [ 0 ] ) {
          state->camera_y -= State::c_camera_speed * time_delta;
     }

     if ( state->camera_direction_keys [ 1 ] ) {
          state->camera_y += State::c_camera_speed * time_delta;
     }

     if ( state->camera_direction_keys [ 2 ] ) {
          state->camera_x -= State::c_camera_speed * time_delta;
     }

     if ( state->camera_direction_keys [ 3 ] ) {
          state->camera_x += State::c_camera_speed * time_delta;
     }
}

static Void render_current_tile ( SDL_Surface* back_buffer, SDL_Surface* tilesheet,
                                  Int32 mouse_x, Int32 mouse_y, int current_tile )
{
     SDL_Rect tile_rect { mouse_x, back_buffer->h - mouse_y,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { current_tile * bryte::Map::c_tile_dimension_in_pixels, 0,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( tilesheet, &clip_rect, back_buffer, &tile_rect );
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     State* state = get_state ( game_memory );

     render_map ( back_buffer, state->tilesheet, state->map, state->camera_x, state->camera_y );

     render_current_tile ( back_buffer, state->tilesheet, state->mouse_x, state->mouse_y,
                           state->current_tile );
}

