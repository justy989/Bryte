#include "Editor.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "MapDisplay.hpp"

using namespace editor;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

extern "C" Bool game_init ( GameMemory& game_memory )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State);

     memory_locations->state = state;

     Uint32 map_width  = 14;
     Uint32 map_height = 14;

     state->room.initialize ( map_width, map_height,
                              GAME_PUSH_MEMORY_ARRAY ( game_memory, Uint8,
                                                       map_width * map_height ) );

     state->map.set_current_room ( &state->room );

     FileContents bitmap_contents = load_entire_file ( "castle_tilesheet.bmp", &game_memory );
     state->tilesheet = load_bitmap ( &bitmap_contents );
     if ( !state->tilesheet ) {
          return false;
     }

     state->current_tile = 1;

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{

}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     State* state = get_state ( game_memory );

     for ( Uint32 i = 0; i < game_input.mouse_button_change_count; ++i ) {
          auto change = game_input.mouse_button_changes [ i ];
          if ( change.down && ( SDL_BUTTON(SDL_BUTTON_LEFT) & change.button ) ) {
               Int32 tx = game_input.mouse_position_x / bryte::Map::c_tile_dimension_in_pixels;
               Int32 ty = game_input.mouse_position_y / bryte::Map::c_tile_dimension_in_pixels;

               state->map.set_coordinate_value ( tx, ty, state->current_tile );
          }
     }

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
          }
     }
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{

}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     State* state = get_state ( game_memory );

     render_map ( back_buffer, state->tilesheet, state->map, 0.0f, 0.0f );
}

