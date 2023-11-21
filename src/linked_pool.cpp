#include "kami/linked_pool.hpp"
#include "kami/linked_polygon.hpp"
#include <sstream>

namespace kami {

// ==========================================================================
// Facet Pool Logic
// ==========================================================================

LinkedMeshPool::LinkedMeshPool(microstl::Mesh &mesh)
    : std::vector<std::shared_ptr<ILinkedMesh>>(mesh.facets.size()) {
  for (ulong i = 0; i < mesh.facets.size(); i++) {
    (*this)[i] = std::make_shared<LinkedTriangle>(&mesh.facets[i], i);
  }
}

void LinkedMeshPool::makeFacetPoolInternalLink() {
  ulong index = 0;
  std::vector<ulong> stack{0};
  while ((index < stack.size()) && (index < this->size())) {
    auto created = (*this)[stack[index]]->linkNeighbours(*this);
    stack.insert(stack.end(), created.begin(), created.end());
    index++;
  }
}

Bounds LinkedMeshPool::getBounds() const {
  Bounds b;
  for (ulong i = 0; i < this->size(); i++)
    b += (*this)[i]->getBounds();
  return b;
}

std::string LinkedMeshPool::getAsSVGString(int max_depth,
                                           double scale_factor) const {
  std::stringstream ss;
  auto b = getBounds();
  ss << "<svg width=\"" << (b.xmax - b.xmin) * scale_factor << "\"";
  ss << " height=\"" << (b.ymax - b.ymin) * scale_factor << "\"";
  ss << " viewBox=\"" << b.xmin * scale_factor << " " << b.ymin * scale_factor
     << " " << (b.xmax - b.xmin) * scale_factor << " "
     << (b.ymax - b.ymin) * scale_factor << "\"";
  ss << " xmlns=\"http://www.w3.org/2000/svg\">";
  (*this)[root]->fillSVGString(ss, 0, max_depth, scale_factor);
  ss << "</svg>";
  return ss.str();
}

void LinkedMeshPool::printInformations() const {
  int solo = 0;       // Get Number of solo facets
  int non_owning = 0; // Number of non owning facets
  /*for (ILinkedMesh &facet : *this) {
    if (facet.f12.mesh == nullptr && facet.f23.mesh == nullptr &&
        facet.f31.mesh == nullptr)
      solo++;
    if (!facet.f12.owned && !facet.f23.owned && !facet.f31.owned)
      non_owning++;
  }*/
  std::cout << "Pool Informations" << std::endl;
  std::cout << "\tSolo facets = " << solo << std::endl;
  std::cout << "\tNon Owning facets = " << non_owning << std::endl;
}

} // namespace kami