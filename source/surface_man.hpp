/* surface_man: Manages surface resources by loading and freeing surfaces. If
*              a path is given that has already been loaded, then it will
*              return an existing surface pointer rather than double loading.
*              When the surface is freed the same number of times it was
*              loaded, then it will be freed completely from memory.
*/

#ifndef BRYTE_SURFACE_MAN_HPP
#define BRYTE_SURFACE_MAN_HPP

#include <SDL2/SDL.h>

#include <string>
#include <unordered_map>
#include <memory>

namespace bryte
{
     class surface_man {
     public:

          ~surface_man ( );

          SDL_Surface* load ( const std::string& path );
          void free ( SDL_Surface* surface );

     private:

          struct surface_resource {
               surface_resource ( const std::string& path, SDL_Surface* surface );

               std::string path;
               SDL_Surface* surface;
               std::size_t owner_count;
          };

          using resource_ptr = std::shared_ptr < surface_resource >;

     private:

          std::unordered_map< SDL_Surface*, resource_ptr > m_surface_resource_map;
          std::unordered_map< std::string, resource_ptr > m_string_resource_map;
     };
}

#endif
