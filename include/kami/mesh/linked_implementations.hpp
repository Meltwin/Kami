#ifndef KAMI_LINKED_POLYGON
#define KAMI_LINKED_POLYGON

#include "kami/mesh/linked_poly.hpp"
namespace kami {

struct LinkedTriangle : public LinkedPolygon {
  LinkedTriangle();
  LinkedTriangle(microstl::Facet *facet, ulong _id);
};

} // namespace kami

#endif