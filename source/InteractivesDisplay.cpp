#include "InteractivesDisplay.hpp"
#include "Map.hpp"

using namespace bryte;

InteractivesDisplay::InteractivesDisplay ( ) :
     interactive_sheet ( nullptr )
{

}

Void InteractivesDisplay::tick ( )
{
     torch_update_delay--;

     if ( torch_update_delay <= 0 ) {
          torch_frame++;
          torch_frame %= c_torch_frame_count;
          torch_update_delay = c_torch_frames_per_update;
     }
}

Void InteractivesDisplay::render ( SDL_Surface* back_buffer, Interactives& interactives,
                                   Real32 camera_x, Real32 camera_y )
{
     for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
               Int32 position_x = x * Map::c_tile_dimension_in_pixels;
               Int32 position_y = y * Map::c_tile_dimension_in_pixels;

               Auto& interactive = interactives.get_from_tile ( x, y );

               render_underneath ( back_buffer, interactive.underneath, position_x, position_y,
                                   camera_x, camera_y );
               render_interactive ( back_buffer, interactive, position_x, position_y, camera_x, camera_y );
          }
     }
}

Void InteractivesDisplay::render_underneath ( SDL_Surface* back_buffer, UnderneathInteractive& underneath,
                                              Int32 position_x, Int32 position_y,
                                              Real32 camera_x, Real32 camera_y )
{

     SDL_Rect dest_rect { position_x, position_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { 0, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     switch ( underneath.type ) {
     default:
          ASSERT ( 0 );
          break;
     case UnderneathInteractive::Type::none:
          return;
     case UnderneathInteractive::Type::pressure_plate:
          if ( underneath.underneath_pressure_plate.entered ) {
               clip_rect.x += Map::c_tile_dimension_in_pixels;
          }
          clip_rect.y = ( Interactive::Type::exit + 2 ) + ( underneath.type - 1 );
          clip_rect.y *= Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::popup_block:
          if ( underneath.underneath_popup_block.up ) {
               clip_rect.y = ( Interactive::Type::pushable_block - 1 ) * Map::c_tile_dimension_in_pixels;
          } else {
               return;
          }
          break;
     }

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( interactive_sheet, &clip_rect, back_buffer, &dest_rect );
}

Void InteractivesDisplay::render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                               Int32 position_x, Int32 position_y,
                                               Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect { position_x, position_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { 0, ( interactive.type - 1 ) * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     switch ( interactive.type ) {
     default:
          ASSERT ( 0 );
          break;
     case Interactive::Type::none:
          return;
     case Interactive::Type::lever:
          if ( interactive.interactive_lever.state == Lever::State::on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels * 2;
          } else if ( interactive.interactive_lever.state == Lever::State::changing_on ||
                      interactive.interactive_lever.state == Lever::State::changing_off ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels;
          }
          break;
     case Interactive::Type::pushable_block:
          break;
     case Interactive::Type::torch:
          if ( interactive.interactive_torch.on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels * ( 1 + torch_frame );
          }
          break;
     case Interactive::Type::pushable_torch:
          if ( interactive.interactive_pushable_torch.torch.on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels * ( 1 + torch_frame );
          }
          break;
     case Interactive::Type::light_detector:
          if ( interactive.interactive_light_detector.type == LightDetector::Type::bryte ) {
               if ( !interactive.interactive_light_detector.below_value ) {
                    clip_rect.x += Map::c_tile_dimension_in_pixels;
               }
          } else {
               clip_rect.x = 2 * Map::c_tile_dimension_in_pixels;

               if ( interactive.interactive_light_detector.below_value ) {
                    clip_rect.x += Map::c_tile_dimension_in_pixels;
               }
          }
          break;
     case Interactive::Type::exit:
          clip_rect.x = interactive.interactive_exit.direction * Map::c_tile_dimension_in_pixels;
          clip_rect.y += interactive.interactive_exit.state * Map::c_tile_dimension_in_pixels;
          break;
     }

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( interactive_sheet, &clip_rect, back_buffer, &dest_rect );
}

