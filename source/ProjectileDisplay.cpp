#include "ProjectileDisplay.hpp"
#include "Map.hpp"
#include "GameMemory.hpp"
#include "Bitmap.hpp"

using namespace bryte;

Bool ProjectileDisplay::load_surfaces ( GameMemory& game_memory )
{
     if ( !load_bitmap_with_game_memory ( arrow_sheet, game_memory, "content/images/test_arrow.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( goo_sheet, game_memory, "content/images/test_goo_proj.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( ice_sheet, game_memory, "content/images/test_ice_proj.bmp" ) ) {
          return false;
     }

     return true;
}

Void ProjectileDisplay::unload_surfaces ( )
{
     FREE_SURFACE ( arrow_sheet );
     FREE_SURFACE ( goo_sheet );
     FREE_SURFACE ( ice_sheet );
}

Void ProjectileDisplay::tick ( )
{
     animation.update_increment ( c_frame_delay, c_frame_count );
}

Void ProjectileDisplay::render ( SDL_Surface* back_buffer, const Projectile& projectile,
                                 Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect = build_world_sdl_rect ( projectile.position.x ( ),
                                                 projectile.position.y ( ),
                                                 Map::c_tile_dimension_in_meters,
                                                 Map::c_tile_dimension_in_meters );

     SDL_Surface* projectile_sheet = nullptr;
     Int32 frame = animation.frame;

     switch ( projectile.type ) {
     default:
          ASSERT ( 0 );
          break;
     case Projectile::Type::arrow:
          projectile_sheet = arrow_sheet;
          if ( !projectile.effected_by_element ) {
               frame = 0;
          } else {
               frame++;
          }
          break;
     case Projectile::Type::goo:
          projectile_sheet = goo_sheet;
          break;
     case Projectile::Type::ice:
          projectile_sheet = ice_sheet;
          break;
     }

     if ( projectile.effected_by_element == Element::ice ) {
          frame += c_frame_count;
     }

     SDL_Rect clip_rect { frame * Map::c_tile_dimension_in_pixels,
                          static_cast<Int32>( projectile.facing ) * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( projectile_sheet, &clip_rect, back_buffer, &dest_rect );
}

