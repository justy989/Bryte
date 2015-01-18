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

static Void render_fixture ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet, Map::Fixture* fixture,
                             Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { fixture->id * Map::c_tile_dimension_in_pixels, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     dest_rect.x = fixture->location_x * Map::c_tile_dimension_in_pixels;
     dest_rect.y = fixture->location_y * Map::c_tile_dimension_in_pixels;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fixture_sheet, &clip_rect, back_buffer, &dest_rect );
}

extern "C" Void render_map_decor ( SDL_Surface* back_buffer, SDL_Surface* decor_sheet, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.decor_count ( ); ++i ) {
          render_fixture ( back_buffer, decor_sheet, &map.decor ( i ), camera_x, camera_y );
     }

}

extern "C" Void render_map_lamps ( SDL_Surface* back_buffer, SDL_Surface* lamp_sheet, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
          render_fixture ( back_buffer, lamp_sheet, &map.lamp ( i ), camera_x, camera_y );
     }
}

extern "C" Void render_map_exits ( SDL_Surface* back_buffer, SDL_Surface* exit_sheet, Map& map,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.exit_count ( ); ++i ) {
          render_fixture ( back_buffer, exit_sheet, &map.exit ( i ), camera_x, camera_y );
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

     // loop over the clamped region
     for ( Int32 y = min_y; y < max_y; ++y ) {
          for ( Int32 x = min_x; x < max_x; ++x ) {

               // get the current pixel location
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

     // we are done accessing the back buffer's pixels directly
     SDL_UnlockSurface ( back_buffer );
}

