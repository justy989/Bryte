#include "Pickup.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Pickup::c_dimension_in_meters = pixels_to_meters ( Pickup::c_dimension_in_pixels );

const Char8* Pickup::c_names [ Type::count ] = { "NONE", "HEALTH", "KEY", "INGREDIENT" };

