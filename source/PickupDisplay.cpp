#include "PickupDisplay.hpp"
#include "Utils.hpp"
#include "GameMemory.hpp"
#include "Bitmap.hpp"

using namespace bryte;

Bool PickupDisplay::load_surfaces ( GameMemory& game_memory )
{
     if ( !load_bitmap_with_game_memory ( pickup_sheet, game_memory, "content/images/test_pickups.bmp" ) ) {
          return false;
     }

     return true;
}

Void PickupDisplay::unload_surfaces ( )
{
     FREE_SURFACE ( pickup_sheet );
}

Void PickupDisplay::tick ( )
{
     animation.update_increment ( c_pickup_animation_delay, c_pickup_animation_max_frame );
}

Void PickupDisplay::render ( SDL_Surface* back_buffer, const Pickup& pickup, Real32 camera_x, Real32 camera_y )
{
     if ( pickup.type == Pickup::Type::none ||
          pickup.type == Pickup::Type::ingredient ) {
          return;
     }

     SDL_Rect dest_rect = build_world_sdl_rect ( pickup.position.x ( ), pickup.position.y ( ),
                                                 Pickup::c_dimension_in_meters,
                                                 Pickup::c_dimension_in_meters );

     SDL_Rect clip_rect { animation.frame * Pickup::c_dimension_in_pixels,
                          ( static_cast<Int32>( pickup.type ) - 1) * Pickup::c_dimension_in_pixels,
                          Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( pickup_sheet, &clip_rect, back_buffer, &dest_rect );
}

