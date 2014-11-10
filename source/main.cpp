#include "game.hpp"

#include <iostream>
#include <stdexcept>

int main ( int argc, char** argv )
{
     try {
          bryte::game g ( argc, argv );
          return g.run ( );
     } catch ( const std::exception& e ) {
          std::cout << "Error: " << e.what () << std::endl;
     }

     return -1;
}

