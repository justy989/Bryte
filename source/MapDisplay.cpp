#include "MapDisplay.hpp"
#include "Utils.hpp"
#include "GameMemory.hpp"
#include "Bitmap.hpp"

using namespace bryte;

Void MapDisplay::clear ( )
{
     tilesheet = nullptr;
     decorsheet = nullptr;
     lampsheet = nullptr;
}

Bool MapDisplay::load_surfaces ( GameMemory& game_memory, const Char8* tilesheet_filepath,
                                 const Char8* decorsheet_filepath, const Char8* lampsheet_filepath )
{
     if ( !load_bitmap_with_game_memory ( tilesheet, game_memory, tilesheet_filepath ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( decorsheet, game_memory, decorsheet_filepath ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( lampsheet, game_memory, lampsheet_filepath ) ) {
          return false;
     }

     return true;
}

Void MapDisplay::unload_surfaces ( )
{
     FREE_SURFACE ( tilesheet );
     FREE_SURFACE ( decorsheet );
     FREE_SURFACE ( lampsheet );
}

static Void render_map_with_invisibles ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                                         Real32 camera_x, Real32 camera_y )
{
     // TODO: optimize to only draw to the part of the back buffer we can see
     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {
               Location tile ( x, y );
               Auto tile_value = map.get_tile_location_value ( tile );

               if ( !tile_value ) {
                    continue;
               }

               tile_value--;

               SDL_Rect tile_rect { 0, 0,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
               SDL_Rect clip_rect { 0, 0,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

               clip_rect.x = tile_value * Map::c_tile_dimension_in_pixels;

               tile_rect.x = tile.x * Map::c_tile_dimension_in_pixels;
               tile_rect.y = tile.y * Map::c_tile_dimension_in_pixels;

               world_to_sdl ( tile_rect, back_buffer, camera_x, camera_y );

               SDL_BlitSurface ( tilesheet, &clip_rect, back_buffer, &tile_rect );
          }
     }
}

static Void render_map ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                         Real32 camera_x, Real32 camera_y )
{
     // TODO: optimize to only draw to the part of the back buffer we can see
     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {
               Location tile ( x, y );

               Auto tile_value = map.get_tile_location_value ( tile );

               if ( !tile_value || map.get_tile_location_invisible ( tile )  ) {
                    continue;
               }

               tile_value--;

               SDL_Rect tile_rect { 0, 0,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
               SDL_Rect clip_rect { 0, 0,
                                    Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

               clip_rect.x = tile_value * Map::c_tile_dimension_in_pixels;

               tile_rect.x = x * Map::c_tile_dimension_in_pixels;
               tile_rect.y = y * Map::c_tile_dimension_in_pixels;

               world_to_sdl ( tile_rect, back_buffer, camera_x, camera_y );

               SDL_BlitSurface ( tilesheet, &clip_rect, back_buffer, &tile_rect );
          }
     }
}

static Void render_fixture ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet, Map::Fixture* fixture,
                             Map& map, Real32 camera_x, Real32 camera_y )
{
     Location tile ( fixture->coordinates.x, fixture->coordinates.y );

     if ( map.get_tile_location_invisible ( tile ) ) {
          return;
     }

     SDL_Rect dest_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { fixture->id * Map::c_tile_dimension_in_pixels, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     dest_rect.x = fixture->coordinates.x * Map::c_tile_dimension_in_pixels;
     dest_rect.y = fixture->coordinates.y * Map::c_tile_dimension_in_pixels;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fixture_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_fixture_with_invisibles ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet,
                                             Map::Fixture* fixture, Map& map, Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { fixture->id * Map::c_tile_dimension_in_pixels, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     dest_rect.x = fixture->coordinates.x * Map::c_tile_dimension_in_pixels;
     dest_rect.y = fixture->coordinates.y * Map::c_tile_dimension_in_pixels;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fixture_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_map_decor ( SDL_Surface* back_buffer, SDL_Surface* decor_sheet, Map& map,
                               Real32 camera_x, Real32 camera_y, Bool invisibles )
{
     if ( invisibles ) {
          for ( Uint8 i = 0; i < map.decor_count ( ); ++i ) {
               render_fixture_with_invisibles ( back_buffer, decor_sheet, &map.decor ( i ), map,
                                                camera_x, camera_y );
          }
     } else {
          for ( Uint8 i = 0; i < map.decor_count ( ); ++i ) {
               render_fixture ( back_buffer, decor_sheet, &map.decor ( i ), map, camera_x, camera_y );
          }
     }
}

static Void render_map_lamps ( SDL_Surface* back_buffer, SDL_Surface* lamp_sheet, Map& map,
                               Real32 camera_x, Real32 camera_y, Bool invisibles )
{
     if ( invisibles ) {
          for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
               render_fixture_with_invisibles ( back_buffer, lamp_sheet, &map.lamp ( i ), map,
                                                camera_x, camera_y );
          }
     } else {
          for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
               render_fixture ( back_buffer, lamp_sheet, &map.lamp ( i ), map, camera_x, camera_y );
          }
     }
}

Void MapDisplay::render ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y,
                          Bool invisibles )
{
     if ( invisibles ) {
          render_map_with_invisibles ( back_buffer, tilesheet, map, camera_x, camera_y );
     } else {
          render_map ( back_buffer, tilesheet, map, camera_x, camera_y );
     }

     render_map_decor ( back_buffer, decorsheet, map, camera_x, camera_y, invisibles );
     render_map_lamps ( back_buffer, lampsheet, map, camera_x, camera_y, invisibles );
}

static Void blend_light ( SDL_Surface* back_buffer, Int32 pixel_x, Int32 pixel_y, Real32 light )
{
     if ( pixel_x > back_buffer->w || pixel_y > back_buffer->h ) {
          return;
     }

     // get the current pixel location
     Uint32* p_pixel = reinterpret_cast<Uint32*>( back_buffer->pixels ) + pixel_x + ( pixel_y * back_buffer->w );

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

extern "C" Void render_light ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y )
{
     // Lock the backbuffer, we are going to access it's pixels
     if ( SDL_LockSurface ( back_buffer ) ) {
          return;
     }

     // TODO: optimize to only draw to the part of the back buffer we can see
     for ( Int32 y = 0; y < static_cast<Int32>( map.light_height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.light_width ( ) ); ++x ) {
               Location pixel ( x, y );

               pixel.x += meters_to_pixels ( camera_x );
               pixel.y += meters_to_pixels ( camera_y );
               pixel.y = ( back_buffer->h - pixel.y );

               Int32 pixel_light = map.get_pixel_light ( Location ( x, y ) );
               Real32 normalized_pixel_light = static_cast<Real32>( pixel_light ) / 255.0f;

               blend_light ( back_buffer, pixel.x, pixel.y, normalized_pixel_light );
          }
     }

     // we are done accessing the back buffer's pixels directly
     SDL_UnlockSurface ( back_buffer );
}

