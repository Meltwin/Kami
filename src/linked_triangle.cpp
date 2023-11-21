#include "kami/linked_mesh.hpp"
#include "kami/linked_polygon.hpp"
#include "kami/math.hpp"

namespace kami {

LinkedTriangle::LinkedTriangle(microstl::Facet *_facet, ulong _id)
    : LinkedMesh<3>() {
  n = math::Vertex(_facet->n.x, _facet->n.y, _facet->n.z, 0);
  n.normalize();
  uid = _id;
  facets[0] = LinkedEdge<ILinkedMesh>{_facet->v1, _facet->v2};
  facets[1] = LinkedEdge<ILinkedMesh>{_facet->v2, _facet->v3};
  facets[2] = LinkedEdge<ILinkedMesh>{_facet->v3, _facet->v1};
}

} // namespace kami