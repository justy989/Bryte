#include "text.hpp"

using namespace bryte;

clipped_sprite* text::ms_sprite = nullptr;
SDL_Surface* text::ms_surface = nullptr;

void text::create_sprite ( )
{
     ms_surface = SDL_LoadBMP ( "text.bmp" );

     ms_sprite = new clipped_sprite ( ms_surface, vector ( ), rectangle ( 0, 0, 0, 0 ) );

     SDL_SetColorKey ( ms_surface, SDL_TRUE, 0xFF00FF );
}

void text::destroy_sprite ( )
{
     SDL_FreeSurface ( ms_surface );
     delete ms_sprite;
}

void text::draw ( SDL_Surface* back_buffer, std::string& message,
                  vector position, color color )
{
     for ( auto character : message ) {
          draw_character ( back_buffer, character, position, color );
          position += vector ( 6, 0 );
     }
}

void text::draw_character ( SDL_Surface* back_buffer, char character,
                            vector position, color color )
{
     auto& clip = ms_sprite->clip ( );
     vector_base_type left = ( character - 'A' ) * 5;
     clip.set ( left, 0, left + 5, 8 );

     ms_sprite->position ( ).set ( position );

     ms_sprite->blit_onto ( back_buffer );
}