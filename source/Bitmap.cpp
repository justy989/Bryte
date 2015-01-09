#include "Bitmap.hpp"
#include "Utils.hpp"

using namespace bryte;

extern "C" SDL_Surface* load_bitmap ( const char* filepath )
{
     LOG_INFO ( "Loading bitmap '%s'\n", filepath );

     FileContents bitmap_contents = load_entire_file ( filepath );

     if ( !bitmap_contents.bytes ) {
          LOG_ERROR ( "Failed to load bitmap '%s'\n", filepath );
          return nullptr;
     }

     BitmapFileHeader* file_header  = reinterpret_cast<BitmapFileHeader*>( bitmap_contents.bytes );
     BitmapInfoHeader* info_header  = reinterpret_cast<BitmapInfoHeader*>( bitmap_contents.bytes +
                                                                           sizeof ( BitmapFileHeader ) );

     LOG_DEBUG ( "'File Header' Type: %c%c Size: %u Bitmap Offset: %u\n",
                 bitmap_contents.bytes [ 0 ], bitmap_contents.bytes [ 1 ],
                 file_header->file_size, file_header->bitmap_offset );
     LOG_DEBUG ( "'Info Header' Size: %u Width: %d Height: %d Bits Per Pixel: %d\n",
                 info_header->size, info_header->width, info_header->height,
                 info_header->bits_per_pixel );

     // do some validation
     if ( bitmap_contents.bytes [ 0 ] != 'B' || bitmap_contents.bytes [ 1 ] != 'M' ) {
          LOG_ERROR ( "Bitmap 'file_type' field '%c%c' but we expected 'BM'. Unknown file format.\n",
                      bitmap_contents.bytes [ 0 ], bitmap_contents.bytes [ 1 ] );
          return nullptr;
     }

     if ( bitmap_contents.size != file_header->file_size ) {
          LOG_WARNING ( "Bitmap 'file_size' %d doesn't match the file size we expected %d\n",
                        file_header->file_size, bitmap_contents.size );
     }

     if ( info_header->size != BitmapInfoHeader::c_size ) {
          LOG_ERROR ( "Info header size %d, expected %d. Unrecognized format\n",
                      info_header->size, BitmapInfoHeader::c_size );
          return nullptr;
     }

     // create a surface to fill with pixels the width and height of the loaded bitmap
     SDL_Surface* surface = SDL_CreateRGBSurface ( 0, info_header->width, info_header->height, 32,
                                                   0, 0, 0, 0 );

     if ( !surface ) {
          LOG_ERROR ( "SDL_CreateRGBSurface() failed: %s\n", SDL_GetError ( ) );
     }

     if ( SDL_LockSurface ( surface ) ) {
          LOG_ERROR ( "SDL_LockSurface() failed: %s\n", SDL_GetError ( ) );
          SDL_FreeSurface ( surface );
          return nullptr;
     }

     Char8* bitmap_pixels   = bitmap_contents.bytes + file_header->bitmap_offset;
     Char8* surface_pixels  = reinterpret_cast<Char8*>( surface->pixels );
     Uint32 bitmap_pitch    = info_header->width * ( info_header->bits_per_pixel / BITS_PER_BYTE );
     Uint32 bytes_per_pixel = info_header->bits_per_pixel / BITS_PER_BYTE;
     Bitscan red_shift      = bitscan_forward ( info_header->red_mask );
     Bitscan green_shift    = bitscan_forward ( info_header->green_mask );
     Bitscan blue_shift     = bitscan_forward ( info_header->blue_mask );

     if ( !red_shift.found || !green_shift.found || !blue_shift.found ) {
          LOG_ERROR ( "Failed to determine shift values, R: %d, G: %d, B: %d\n",
                      red_shift.bit, green_shift.bit, blue_shift.bit );
          SDL_FreeSurface ( surface );
          return nullptr;
     }

     // start at the bottom of the bitmap
     bitmap_pixels += bitmap_pitch * ( info_header->height - 1 );

     for ( Int32 y = 0; y < info_header->height; ++y ) {

          for ( Int32 x = 0; x < info_header->width; ++x ) {
               Uint32 bitmap_pixel = *reinterpret_cast<Uint32*>( bitmap_pixels + ( x * bytes_per_pixel ) );

               *surface_pixels = bitmap_pixel >> blue_shift.bit;  surface_pixels++;
               *surface_pixels = bitmap_pixel >> green_shift.bit; surface_pixels++;
               *surface_pixels = bitmap_pixel >> red_shift.bit;   surface_pixels++;
               *surface_pixels = 255;                             surface_pixels++;
          }

          bitmap_pixels -= bitmap_pitch;
     }

     SDL_UnlockSurface ( surface );

#if 0 // do not set yet
     if ( !SDL_SetColorKey ( surface, SDL_TRUE, c_bitmap_color_key ) ) {
          LOG_ERROR ( "SDL_SetColorKey() Failed: %s\n", SDL_GetError ( ) );
          SDL_FreeSurface ( surface );
          return nullptr;
     }
#endif

     return surface;
}

