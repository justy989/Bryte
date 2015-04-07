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

     lamp_animation.clear ( );
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

Void MapDisplay::tick ( )
{
     lamp_animation.update_increment ( c_lamp_frame_delay, c_lamp_frame_count );
}

static Void render_map_with_invisibles ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                                         Real32 camera_x, Real32 camera_y )
{
     // TODO: optimize to only draw to the part of the back buffer we can see
     for ( Location tile; tile.y < static_cast<Int32>( map.height ( ) ); ++tile.y ) {
          for ( tile.x = 0; tile.x < static_cast<Int32>( map.width ( ) ); ++tile.x ) {
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
     for ( Location tile; tile.y < static_cast<Int32>( map.height ( ) ); ++tile.y ) {
          for ( tile.x = 0; tile.x < static_cast<Int32>( map.width ( ) ); ++tile.x ) {
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

               tile_rect.x = tile.x * Map::c_tile_dimension_in_pixels;
               tile_rect.y = tile.y * Map::c_tile_dimension_in_pixels;

               world_to_sdl ( tile_rect, back_buffer, camera_x, camera_y );

               SDL_BlitSurface ( tilesheet, &clip_rect, back_buffer, &tile_rect );
          }
     }
}

static Void render_decor ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet, Map::Fixture* fixture,
                           Map& map, Real32 camera_x, Real32 camera_y )
{
     Location tile ( fixture->coordinates );

     if ( map.get_tile_location_invisible ( tile ) ) {
          return;
     }

     SDL_Rect dest_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { fixture->id * Map::c_tile_dimension_in_pixels, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     dest_rect.x = tile.x * Map::c_tile_dimension_in_pixels;
     dest_rect.y = tile.y * Map::c_tile_dimension_in_pixels;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fixture_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_decor_with_invisibles ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet,
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
               render_decor_with_invisibles ( back_buffer, decor_sheet, &map.decor ( i ), map,
                                              camera_x, camera_y );
          }
     } else {
          for ( Uint8 i = 0; i < map.decor_count ( ); ++i ) {
               render_decor ( back_buffer, decor_sheet, &map.decor ( i ), map, camera_x, camera_y );
          }
     }
}

static Void render_lamp ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet, Map::Fixture* fixture,
                          Map& map, Real32 camera_x, Real32 camera_y, Int32 lamp_frame )
{
     Location tile ( fixture->coordinates );

     if ( map.get_tile_location_invisible ( tile ) ) {
          return;
     }

     SDL_Rect dest_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { lamp_frame * Map::c_tile_dimension_in_pixels,
                          fixture->id * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     dest_rect.x = tile.x * Map::c_tile_dimension_in_pixels;
     dest_rect.y = tile.y * Map::c_tile_dimension_in_pixels;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fixture_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_lamp_with_invisibles ( SDL_Surface* back_buffer, SDL_Surface* fixture_sheet,
                                          Map::Fixture* fixture, Map& map, Real32 camera_x, Real32 camera_y,
                                          Int32 lamp_frame )
{
     SDL_Rect dest_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { lamp_frame * Map::c_tile_dimension_in_pixels,
                          fixture->id * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     dest_rect.x = fixture->coordinates.x * Map::c_tile_dimension_in_pixels;
     dest_rect.y = fixture->coordinates.y * Map::c_tile_dimension_in_pixels;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fixture_sheet, &clip_rect, back_buffer, &dest_rect );
}


static Void render_map_lamps ( SDL_Surface* back_buffer, SDL_Surface* lamp_sheet, Map& map,
                               Real32 camera_x, Real32 camera_y, Bool invisibles,
                               Int32 lamp_frame )
{
     if ( invisibles ) {
          for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
               render_lamp_with_invisibles ( back_buffer, lamp_sheet, &map.lamp ( i ), map,
                                             camera_x, camera_y, lamp_frame );
          }
     } else {
          for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
               render_lamp ( back_buffer, lamp_sheet, &map.lamp ( i ), map, camera_x, camera_y,
                             lamp_frame );
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
     render_map_lamps ( back_buffer, lampsheet, map, camera_x, camera_y, invisibles,
                        lamp_animation.frame );
}

static Void blend_tile_light ( SDL_Surface* back_buffer, const Location& tile_bottom_left_pixel, Real32 light )
{
     Int32 max_x = tile_bottom_left_pixel.x + Map::c_tile_dimension_in_pixels;
     Int32 max_y = tile_bottom_left_pixel.y + Map::c_tile_dimension_in_pixels;

     Uint32* p_pixel = reinterpret_cast<Uint32*>( back_buffer->pixels );
     p_pixel += tile_bottom_left_pixel.x + ( tile_bottom_left_pixel.y * back_buffer->w );

     for ( Location pixel = tile_bottom_left_pixel; pixel.y < max_y; ++pixel.y ) {
          for ( pixel.x = tile_bottom_left_pixel.x; pixel.x < max_x; ++pixel.x ) {
               if ( pixel.x < 0 || pixel.x > back_buffer->w ||
                    pixel.y < 0 || pixel.y > back_buffer->h ) {
                    ++p_pixel;
                    continue;
               }

               // get the current pixel location
               // read pixels
               Uint8 red   = *( reinterpret_cast<Uint8*>( p_pixel ) );
               Uint8 green = *( reinterpret_cast<Uint8*>( p_pixel ) + 1 );
               Uint8 blue  = *( reinterpret_cast<Uint8*>( p_pixel ) + 2 );

               // blend
               Real32 blended_red   = static_cast<Real32>( red ) * light;
               Real32 blended_green = static_cast<Real32>( green ) * light;
               Real32 blended_blue  = static_cast<Real32>( blue ) * light;

               // write pixel
               *( reinterpret_cast<Uint8*>( p_pixel ) )     = static_cast<Uint8>( blended_red );
               *( reinterpret_cast<Uint8*>( p_pixel ) + 1 ) = static_cast<Uint8>( blended_green );
               *( reinterpret_cast<Uint8*>( p_pixel ) + 2 ) = static_cast<Uint8>( blended_blue );

               ++p_pixel;
          }

          p_pixel += back_buffer->w;
          p_pixel -= Map::c_tile_dimension_in_pixels;
     }
}

extern "C" Void render_light ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y )
{
     // Lock the backbuffer, we are going to access it's pixels
     if ( SDL_LockSurface ( back_buffer ) ) {
          return;
     }

     for ( Location tile; tile.y < static_cast<Int32>( map.height ( ) ); ++tile.y ) {
          for ( tile.x = 0; tile.x < static_cast<Int32>( map.width ( ) ); ++tile.x ) {
               Location tile_dest = tile;

               Map::convert_tiles_to_pixels ( &tile_dest );

               tile_dest.x += meters_to_pixels ( camera_x );
               tile_dest.y += meters_to_pixels ( camera_y );
               tile_dest.y = ( back_buffer->h - tile_dest.y ) - Map::c_tile_dimension_in_pixels;

               // ignore off-screen light
               if ( tile_dest.x < -Map::c_tile_dimension_in_pixels ||
                    tile_dest.x > back_buffer->w ||
                    tile_dest.y < -Map::c_tile_dimension_in_pixels ||
                    tile_dest.y > back_buffer->h ) {
                    continue;
               }

               Uint8 pixel_light = map.get_tile_location_light ( tile );
               Real32 normalized_pixel_light = static_cast<Real32>( pixel_light ) / 255.0f;

               blend_tile_light ( back_buffer, tile_dest, normalized_pixel_light );
          }
     }

     // we are done accessing the back buffer's pixels directly
     SDL_UnlockSurface ( back_buffer );
}

