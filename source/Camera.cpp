#include "Camera.hpp"
#include "Utils.hpp"
#include "Map.hpp"

using namespace bryte;

extern "C" Real32 calculate_camera_position ( Int32 back_buffer_dimension, Int32 map_dimension,
                                              Real32 player_position, Real32 player_dimension )
{
     Real32 camera_center_offset   = pixels_to_meters ( back_buffer_dimension / 2 );
     Int32 map_dimension_in_pixels = map_dimension * Map::c_tile_dimension_in_pixels;

     if ( map_dimension_in_pixels < back_buffer_dimension ) {
          return -pixels_to_meters ( map_dimension_in_pixels / 2 ) + camera_center_offset;
     }

     Real32 map_dimension_in_meters = pixels_to_meters ( map_dimension_in_pixels );
     Real32 half_player_dimension   = player_dimension * 0.5f;
     Real32 camera_pos              = -( player_position + half_player_dimension - camera_center_offset );
     Real32 min_camera_pos          = -( map_dimension_in_meters - pixels_to_meters ( back_buffer_dimension ) );

     CLAMP ( camera_pos, min_camera_pos, 0.0f );

     return camera_pos;
}

