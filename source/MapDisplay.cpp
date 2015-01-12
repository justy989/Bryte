#include "MapDisplay.hpp"
#include "Utils.hpp"

using namespace bryte;

extern "C" Void render_map ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                             Real32 camera_x, Real32 camera_y )
{
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
}
extern "C" Void render_map_exits ( SDL_Surface* back_buffer, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     Uint32 exit_color = SDL_MapRGB ( back_buffer->format, 0, 170, 0 );

     for ( Uint8 d = 0; d < map.exit_count ( ); ++d ) {
          auto& exit = map.exit ( d );

          SDL_Rect exit_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

          exit_rect.x = exit.location_x * Map::c_tile_dimension_in_pixels;
          exit_rect.y = exit.location_y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( exit_rect, back_buffer, camera_x, camera_y );

          SDL_FillRect ( back_buffer, &exit_rect, exit_color );
     }
}

