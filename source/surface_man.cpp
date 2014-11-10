#include "surface_man.hpp"

using namespace bryte;

surface_man::~surface_man ( )
{
     for ( std::size_t i = 0; i < m_string_resource_map.bucket_count ( ); ++i ) {
          while ( m_string_resource_map.begin ( i ) != m_string_resource_map.end ( i ) ) {

               auto it = m_string_resource_map.begin ( i );

               // clear owners and free the resource
               it->second->owner_count = 1;
               free ( it->second->surface );
          }
     }
}

SDL_Surface* surface_man::load ( const std::string& path )
{
     // see if the surface has already been loaded
     auto it = m_string_resource_map.find ( path );
 
     if ( it != m_string_resource_map.end ( ) ) {
          it->second->owner_count++;
          return it->second->surface;
     }

     // if we didn't find it, load it and add it to our maps
     SDL_Surface* surface = SDL_LoadBMP ( path.c_str ( ) );

     // if we failed to load it, return nullptr
     if ( !surface ) {
          return nullptr;
     }

     auto ptr = std::make_shared<surface_resource> ( path, surface );

     // add the shared ptr to both maps
     m_string_resource_map [ path ] = ptr;
     m_surface_resource_map [ surface ] = ptr;

     return surface;
}

void surface_man::free ( SDL_Surface* surface )
{
     // see if the surface exists
     auto surf_it = m_surface_resource_map.find ( surface );

     if ( surf_it == m_surface_resource_map.end ( ) ) {
          return;
     }

     // kill an owner
     surf_it->second->owner_count--;

     // if there are no more owners, kill from both maps
     if ( surf_it->second->owner_count == 0 ) {
          m_string_resource_map.erase ( surf_it->second->path );
          m_surface_resource_map.erase ( surf_it );
     }
}

surface_man::surface_resource::surface_resource ( const std::string& path, SDL_Surface* surface ) :
     path ( path ),
     surface ( surface ),
     owner_count ( 1 )
{

}
