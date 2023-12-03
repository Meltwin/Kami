#include "kami/mesh/linked_implementations.hpp"
#include "kami/mesh/linked_poly.hpp"

namespace kami {

LinkedTriangle::LinkedTriangle(microstl::Facet *_facet, ulong _id)
    : LinkedPolygon() {
  n = math::Vertex(_facet->n.x, _facet->n.y, _facet->n.z, 0);
  n.normalize();
  uid = _id;
  facets[0] = LinkedEdge<LinkedPolygon>{_facet->v1, _facet->v2};
  facets[1] = LinkedEdge<LinkedPolygon>{_facet->v2, _facet->v3};
  facets[2] = LinkedEdge<LinkedPolygon>{_facet->v3, _facet->v1};
}

} // namespace kami