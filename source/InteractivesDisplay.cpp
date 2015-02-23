#include "InteractivesDisplay.hpp"
#include "Map.hpp"

using namespace bryte;

InteractivesDisplay::InteractivesDisplay ( ) :
     interactive_sheet ( nullptr ),
     ice_sleep_counter ( 0 )
{

}

Void InteractivesDisplay::tick ( )
{
     animation.update_increment ( c_frames_per_update );
     moving_walkway_animation.update_increment ( c_moving_walkway_frame_delay, c_moving_walkway_frame_count );

     ice_sleep_counter++;

     if ( ice_sleep_counter > c_ice_frame_sleep ) {
          Int32 current_frame = ice_animation.frame;

          ice_animation.update_increment ( c_ice_frame_delay, c_ice_frame_count );

          // check for when we wrap around
          if ( current_frame > ice_animation.frame ) {
               ice_sleep_counter = 0;
               ice_animation.frame = 0;
               ice_animation.delay_tracker = 0;
          }
     }
}

Void InteractivesDisplay::render ( SDL_Surface* back_buffer, Interactives& interactives,
                                   const Map& map, Real32 camera_x, Real32 camera_y, Bool invisible )
{
     if ( invisible ) {
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
     } else {
          for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
               for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
                    Int32 position_x = x * Map::c_tile_dimension_in_pixels;
                    Int32 position_y = y * Map::c_tile_dimension_in_pixels;

                    if ( map.get_coordinate_invisible ( x, y ) ) {
                         continue;
                    }

                    Auto& interactive = interactives.get_from_tile ( x, y );

                    render_underneath ( back_buffer, interactive.underneath, position_x, position_y,
                                        camera_x, camera_y );
                    render_interactive ( back_buffer, interactive, position_x, position_y, camera_x, camera_y );
               }
          }
     }
}

Void InteractivesDisplay::render_underneath ( SDL_Surface* back_buffer, UnderneathInteractive& underneath,
                                              Int32 position_x, Int32 position_y,
                                              Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect { position_x, position_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { 0, ( ( Interactive::Type::count - 2 ) + underneath.type ) * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_Surface* underneath_sheet = interactive_sheet;

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
          break;
     case UnderneathInteractive::Type::popup_block:
          if ( underneath.underneath_popup_block.up ) {
               clip_rect.y = ( Interactive::Type::pushable_block - 1 ) * Map::c_tile_dimension_in_pixels;
          } else {
               return;
          }
          break;
     case UnderneathInteractive::Type::ice:
          clip_rect.x = ice_animation.frame * Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::moving_walkway:
          underneath_sheet = moving_walkway_sheet;
          clip_rect.x = moving_walkway_animation.frame * Map::c_tile_dimension_in_pixels;
          clip_rect.y = underneath.underneath_moving_walkway.facing * Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::light_detector:
          underneath_sheet = light_detector_sheet;
          clip_rect.y = 0;

          if ( underneath.underneath_light_detector.type == LightDetector::Type::bryte ) {
               if ( !underneath.underneath_light_detector.below_value ) {
                    Int32 detector_frame = 1 + ( animation.frame % c_light_detector_frame_count );
                    clip_rect.x += Map::c_tile_dimension_in_pixels * detector_frame;
               }
          } else {
               clip_rect.y += Map::c_tile_dimension_in_pixels;

               if ( underneath.underneath_light_detector.below_value ) {
                    Int32 detector_frame = 1 + ( animation.frame % c_light_detector_frame_count );
                    clip_rect.x += Map::c_tile_dimension_in_pixels * detector_frame;
               }
          }
          break;
     case UnderneathInteractive::Type::ice_detector:
          clip_rect.x = underneath.underneath_ice_detector.detected * Map::c_tile_dimension_in_pixels;
          break;
     }

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( underneath_sheet, &clip_rect, back_buffer, &dest_rect );
}

Void InteractivesDisplay::render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                               Int32 position_x, Int32 position_y,
                                               Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect { position_x, position_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { 0, ( interactive.type - 1 ) * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_Surface* sheet = interactive_sheet;

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
          break;
     case Interactive::Type::pushable_torch:
          break;
     case Interactive::Type::exit:
          sheet = exit_sheet;
          clip_rect.y = 0;
          clip_rect.x = interactive.interactive_exit.direction * Map::c_tile_dimension_in_pixels;

          switch ( interactive.interactive_exit.state ) {
          default:
               break;
          case Exit::State::closed:
               break;
          case Exit::State::open:
               clip_rect.y += 2 * Map::c_tile_dimension_in_pixels;
               break;
          case Exit::State::locked:
               clip_rect.y += 4 * Map::c_tile_dimension_in_pixels;
               break;
          case Exit::State::changing_to_closed:
          case Exit::State::changing_to_open:
               clip_rect.y += Map::c_tile_dimension_in_pixels;
               break;
          case Exit::State::changing_to_locked:
          case Exit::State::changing_to_unlocked:
               clip_rect.y += 3 * Map::c_tile_dimension_in_pixels;
               break;
          }

          break;
     case Interactive::Type::bombable_block:
          break;
     case Interactive::Type::turret:
          clip_rect.x = interactive.interactive_turret.facing * Map::c_tile_dimension_in_pixels;
          break;
     }

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( sheet, &clip_rect, back_buffer, &dest_rect );

     switch ( interactive.type ) {
     default:
          break;
     case Interactive::Type::torch:
          if ( interactive.interactive_torch.element ) {
               Int32 torch_frame = animation.frame % c_torch_frame_count;
               Int32 torch_row = interactive.interactive_torch.element;

               torch_row -= 1;
               torch_row *= Map::c_tile_dimension_in_pixels;

               clip_rect.x = Map::c_tile_dimension_in_pixels * torch_frame;
               clip_rect.y = torch_row;

               SDL_BlitSurface ( torch_element_sheet, &clip_rect, back_buffer, &dest_rect );
          }
          break;
     case Interactive::Type::pushable_torch:
          if ( interactive.interactive_pushable_torch.torch.element ) {
               // TODO: clearly the torch and pushabled torch have the same element display code
               Int32 torch_frame = animation.frame % c_torch_frame_count;
               Int32 torch_row = interactive.interactive_torch.element;

               torch_row -= 1;
               torch_row *= Map::c_tile_dimension_in_pixels;

               clip_rect.x = Map::c_tile_dimension_in_pixels * torch_frame;
               clip_rect.y = torch_row;
               SDL_BlitSurface ( torch_element_sheet, &clip_rect, back_buffer, &dest_rect );
          }
          break;
     }
}

