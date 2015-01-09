#include "Bitmap.hpp"
#include "Utils.hpp"
#include "Globals.hpp"

using namespace bryte;

static Bool read_bitmap_headers ( const FileContents& bitmap_contents, BitmapFileHeader** file_header,
                                  BitmapInfoHeader** info_header )
{
     auto* bitmap_file_header  = reinterpret_cast<BitmapFileHeader*>( bitmap_contents.bytes );
     auto* bitmap_info_header  = reinterpret_cast<BitmapInfoHeader*>( bitmap_contents.bytes +
                                                                      sizeof ( BitmapFileHeader ) );

     LOG_DEBUG ( "'File Header' Type: %c%c Size: %u Bitmap Offset: %u\n",
                 bitmap_contents.bytes [ 0 ], bitmap_contents.bytes [ 1 ],
                 bitmap_file_header->file_size, bitmap_file_header->bitmap_offset );
     LOG_DEBUG ( "'Info Header' Size: %u Width: %d Height: %d Bits Per Pixel: %d\n",
                 bitmap_info_header->size, bitmap_info_header->width, bitmap_info_header->height,
                 bitmap_info_header->bits_per_pixel );

     // do some validation
     if ( bitmap_contents.bytes [ 0 ] != 'B' || bitmap_contents.bytes [ 1 ] != 'M' ) {
          LOG_ERROR ( "Bitmap 'file_type' field '%c%c' but we expected 'BM'. Unknown file format.\n",
                      bitmap_contents.bytes [ 0 ], bitmap_contents.bytes [ 1 ] );
          return false;
     }

     if ( bitmap_contents.size != bitmap_file_header->file_size ) {
          LOG_WARNING ( "Bitmap 'file_size' %d doesn't match the file size we expected %d\n",
                        bitmap_file_header->file_size, bitmap_contents.size );
     }

     if ( bitmap_info_header->size != BitmapInfoHeader::c_size ) {
          LOG_ERROR ( "Info header size %d, expected %d. Unrecognized format\n",
                      bitmap_info_header->size, BitmapInfoHeader::c_size );
          return false;
     }

     *file_header = bitmap_file_header;
     *info_header = bitmap_info_header;

     return true;
}

static Bool fill_surface_pixels ( char* bitmap_pixels, BitmapInfoHeader* info_header, SDL_Surface* surface )
{
     if ( SDL_LockSurface ( surface ) ) {
          LOG_ERROR ( "SDL_LockSurface() failed: %s\n", SDL_GetError ( ) );
          return false;
     }

     Char8* surface_pixels  = reinterpret_cast<Char8*>( surface->pixels );
     Uint32 bitmap_pitch    = info_header->width * ( info_header->bits_per_pixel / BITS_PER_BYTE );
     Uint32 bytes_per_pixel = info_header->bits_per_pixel / BITS_PER_BYTE;
     Bitscan red_shift      = bitscan_forward ( info_header->red_mask );
     Bitscan green_shift    = bitscan_forward ( info_header->green_mask );
     Bitscan blue_shift     = bitscan_forward ( info_header->blue_mask );

     if ( !red_shift.found || !green_shift.found || !blue_shift.found ) {
          LOG_ERROR ( "Failed to determine shift values, R: %d, G: %d, B: %d\n",
                      red_shift.bit, green_shift.bit, blue_shift.bit );
          return false;
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

     return true;
}

extern "C" SDL_Surface* load_bitmap ( const char* filepath )
{
     LOG_DEBUG ( "Loading bitmap '%s'\n", filepath );

     SDL_Surface* surface = nullptr;

     FileContents bitmap_contents = load_entire_file ( filepath );

     if ( !bitmap_contents.bytes ) {
          return surface;
     }

     BitmapFileHeader* file_header = nullptr;
     BitmapInfoHeader* info_header = nullptr;

     if ( !read_bitmap_headers ( bitmap_contents, &file_header, &info_header ) ) {
          bitmap_contents.free ( );
          return surface;
     }

     // create a surface to fill with pixels the width and height of the loaded bitmap
     surface = SDL_CreateRGBSurface ( 0, info_header->width, info_header->height, 32,
                                      0, 0, 0, 0 );

     if ( surface ) {
          if ( !fill_surface_pixels ( bitmap_contents.bytes + file_header->bitmap_offset,
                                      info_header, surface ) ) {
               SDL_FreeSurface ( surface );
               surface = nullptr;
          }
     } else {
          LOG_ERROR ( "SDL_CreateRGBSurface() failed: %s\n", SDL_GetError ( ) );
     }

     bitmap_contents.free ( );

     return surface;
}

