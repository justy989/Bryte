#include "MapDisplay.hpp"
#include "Utils.hpp"

using namespace bryte;

extern "C" Void render_map ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                             Real32 camera_x, Real32 camera_y )
{
     // TODO: optimize to only draw to the part of the back buffer we can see
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

extern "C" Void render_map_decor ( SDL_Surface* back_buffer, SDL_Surface* decorsheet, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.decor_count ( ); ++i ) {
          auto& decor = map.decor ( i );

          SDL_Rect decor_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
          SDL_Rect clip_rect { decor.id * Map::c_tile_dimension_in_pixels, 0,
                               Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

          decor_rect.x = decor.location_x * Map::c_tile_dimension_in_pixels;
          decor_rect.y = decor.location_y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( decor_rect, back_buffer, camera_x, camera_y );

          SDL_BlitSurface ( decorsheet, &clip_rect, back_buffer, &decor_rect );
     }

}

extern "C" Void render_map_lamps ( SDL_Surface* back_buffer, SDL_Surface* lampsheet, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
          auto& lamp = map.lamp ( i );

          SDL_Rect lamp_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
          SDL_Rect clip_rect { lamp.id * Map::c_tile_dimension_in_pixels, 0,
                               Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

          lamp_rect.x = lamp.location_x * Map::c_tile_dimension_in_pixels;
          lamp_rect.y = lamp.location_y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( lamp_rect, back_buffer, camera_x, camera_y );

          SDL_BlitSurface ( lampsheet, &clip_rect, back_buffer, &lamp_rect );
     }
}

extern "C" Void render_map_exits ( SDL_Surface* back_buffer, SDL_Surface* exit_surface, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.exit_count ( ); ++i ) {
          auto& exit = map.exit ( i );

          SDL_Rect exit_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
          SDL_Rect clip_rect { exit.id * Map::c_tile_dimension_in_pixels, 0,
                               Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

          exit_rect.x = exit.location_x * Map::c_tile_dimension_in_pixels;
          exit_rect.y = exit.location_y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( exit_rect, back_buffer, camera_x, camera_y );

          SDL_BlitSurface ( exit_surface, &clip_rect, back_buffer, &exit_rect );
     }
}

static Void blend_light ( SDL_Surface* back_buffer, const SDL_Rect& dest_rect, Real32 light )
{
     Int32 min_x = dest_rect.x;
     Int32 max_x = dest_rect.x + dest_rect.w;
     Int32 min_y = dest_rect.y;
     Int32 max_y = dest_rect.y + dest_rect.h;

     CLAMP ( min_x, 0, back_buffer->w - 1 );
     CLAMP ( min_y, 0, back_buffer->h - 1 );
     CLAMP ( max_x, 0, back_buffer->w - 1 );
     CLAMP ( max_y, 0, back_buffer->h - 1 );

     for ( Int32 y = min_y; y < max_y; ++y ) {
          for ( Int32 x = min_x; x < max_x; ++x ) {
               Uint32* p_pixel = reinterpret_cast<Uint32*>( back_buffer->pixels ) + x + ( y * back_buffer->w );

               // read pixels
               Uint8 red   = *( reinterpret_cast<Uint8*>( p_pixel ) );
               Uint8 green = *( reinterpret_cast<Uint8*>( p_pixel ) + 1 );
               Uint8 blue  = *( reinterpret_cast<Uint8*>( p_pixel ) + 2 );

               // blend
               Real32 blended_red   = static_cast<Real32>( red ) * light;
               Real32 blended_green = static_cast<Real32>( green ) * light;
               Real32 blended_blue  = static_cast<Real32>( blue ) * light;

               // write pixels
               *( reinterpret_cast<Uint8*>( p_pixel ) )     = static_cast<Uint8>( blended_red );
               *( reinterpret_cast<Uint8*>( p_pixel ) + 1 ) = static_cast<Uint8>( blended_green );
               *( reinterpret_cast<Uint8*>( p_pixel ) + 2 ) = static_cast<Uint8>( blended_blue );
          }
     }
}

extern "C" Void render_light ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y )
{
     // Lock the backbuffer, we are going to access it's pixels
     if ( SDL_LockSurface ( back_buffer ) ) {
          return;
     }

     // TODO: optimize to only draw to the part of the back buffer we can see
     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {
               SDL_Rect dest_rect { x * Map::c_tile_dimension_in_pixels,
                                    y * Map::c_tile_dimension_in_pixels,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

               world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

               blend_light ( back_buffer, dest_rect,
                             static_cast<Real32>( map.get_coordinate_light ( x, y ) ) / 255.0f );
          }
     }

     SDL_UnlockSurface ( back_buffer );
}

