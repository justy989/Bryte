#ifndef BRYTE_BITMAP_HPP
#define BRYTE_BITMAP_HPP

#include <SDL2/SDL.h>

#include "Types.hpp"
#include "Utils.hpp"

#pragma pack(1)
struct BitmapFileHeader {
     Uint16 file_type;
     Uint32 file_size;
     Uint16 reserved_1;
     Uint16 reserved_2;
     Uint32 bitmap_offset;
};
#pragma pack()

#pragma pack(1)
// only supporting Bitmap V4, only use fields up to the mask
struct BitmapInfoHeader {

     // expected value of the size field
     static const Uint32 c_size = 124;

     Uint32 size;

     Int32  width;
     Int32  height;

     Uint16 planes;
     Uint16 bits_per_pixel;

     Uint32 compression;
     Uint32 size_of_bitmap;

     Int32  horz_resolution;
     Int32  vert_resolution;

     Uint32 colors_used;
     Uint32 colors_important;

     Uint32 red_mask;
     Uint32 green_mask;
     Uint32 blue_mask;
     Uint32 alpha_mask;
};
#pragma pack()

// save in gimp as 32 bit bitmap, do not have compatibility options checked on
extern "C" SDL_Surface* load_bitmap ( const FileContents* file_contents );

#endif

