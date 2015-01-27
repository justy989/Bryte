#include "InteractivesDisplay.hpp"
#include "Map.hpp"

using namespace bryte;

InteractivesDisplay::InteractivesDisplay ( )
{
     for ( Int32 i = 0; i < Interactive::Type::count; ++i ) {
          interactive_sheets [ i ] = nullptr;
     }
}

Void InteractivesDisplay::render ( SDL_Surface* back_buffer, Interactives& interactives,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
               render_interactive ( back_buffer, interactives.get_from_tile ( x, y ),
                                    x * Map::c_tile_dimension_in_pixels,
                                    y * Map::c_tile_dimension_in_pixels,
                                    camera_x, camera_y );
          }
     }
}

Void InteractivesDisplay::render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                               Int32 position_x, Int32 position_y,
                                               Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect { position_x, position_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { 0, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     switch ( interactive.type ) {
     default:
          ASSERT ( 0 );
          break;
     case Interactive::Type::none:
          return;
     case Interactive::Type::lever:
          if ( interactive.interactive_lever.on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels;
          }
          break;
     case Interactive::Type::pushable_block:
          break;
     case Interactive::Type::exit:
          clip_rect.x = interactive.interactive_exit.direction * Map::c_tile_dimension_in_pixels;
          clip_rect.y = interactive.interactive_exit.state * Map::c_tile_dimension_in_pixels;
          break;
     case Interactive::Type::torch:
          if ( interactive.interactive_torch.on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels;
          }
          break;
     case Interactive::Type::pushable_torch:
          if ( interactive.interactive_pushable_torch.torch.on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels;
          }
          break;
     case Interactive::Type::light_detector:
          if ( interactive.interactive_light_detector.type == LightDetector::Type::bryte ) {
               if ( !interactive.interactive_light_detector.below_value ) {
                    clip_rect.x = Map::c_tile_dimension_in_pixels;
               }
          } else {
               if ( interactive.interactive_light_detector.below_value ) {
                    clip_rect.x = Map::c_tile_dimension_in_pixels;
               }

               clip_rect.y = Map::c_tile_dimension_in_pixels;
          }
          break;
     }

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( interactive_sheets [ interactive.type ], &clip_rect, back_buffer, &dest_rect );
}
