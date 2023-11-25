#include "kami/linked_pool.hpp"
#include "kami/bin_packing.hpp"
#include "kami/bounds.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/linked_polygon.hpp"
#include <cmath>
#include <sstream>
#include <vector>

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

std::vector<bin::Bin<ILinkedMesh>> LinkedMeshPool::slice() {
  std::vector<bin::Box<ILinkedMesh>> boxes;
  (*this)[root]->sliceChildren(*this, boxes);

  // Adding the root to the list
  auto b = (*this)[root]->getBounds(true, true);

  math::HMat mat;
  mat.setTransAsAxis(Vec3{-b.xmin, -b.ymin, 0});
  (*this)[root]->transform(mat, true, true);
  boxes.push_back(bin::Box<ILinkedMesh>((*this)[root].get(),
                                        (*this)[root]->getBounds(true)));

  // Debug
  std::cout << "Got " << boxes.size() << " parts for this mesh" << std::endl;
  for (auto &b : boxes) {
    std::cout << "\tBox(" << b.x << ", " << b.y << ", " << b.getWidth() << ", "
              << b.getHeight() << ")" << std::endl;
  }

  // Launch the bin packing
  return binPackingAlgorithm(boxes, bin::PaperA<4>());
}

std::vector<bin::Bin<ILinkedMesh>>
LinkedMeshPool::binPackingAlgorithm(std::vector<bin::Box<ILinkedMesh>> &boxes,
                                    const bin::BinFormat &format) {
  // Sorting the items by decreasing value
  std::sort(boxes.begin(), boxes.end(),
            [](bin::Box<ILinkedMesh> &elem1, bin::Box<ILinkedMesh> &elem2) {
              return (elem1.height * elem1.width) >
                     (elem2.width * elem2.height);
            });

  // Orient them horizontally
  for (auto &box : boxes) {
    if (box.width < box.height) {
      box.rotated = true;
      double temp = box.width;
      box.width = box.height;
      box.height = box.width;
    }
  }

  // PHASE 1: Compute the lower bound (number of bins to open)
  double L = 0;
  for (auto &box : boxes) {
    L += box.height * box.width;
  }
  int L0 = std::ceil(L / (format.height * format.width));
  std::vector<bin::Bin<ILinkedMesh>> bins(L0, format);

  // PHASE 2: Packing the boxes
  for (auto &box : boxes) {
    double score = 0, temp_score = 0;

    std::cout << "New box packing (" << box.width << ", " << box.height << ")"
              << std::endl;

    // Compute best position
    ulong best_bin = 0;
    ulong best_corner = 0;
    ulong best_rotated = false;
    for (ulong n_bin = 0; n_bin < bins.size(); n_bin++) {
      std::cout << "Bin " << n_bin << " corners :" << std::endl;
      for (ulong n_c = 0; n_c < bins[n_bin].corners.size(); n_c++) {
        std::cout << "\t" << n_c << " -> (" << bins[n_bin].corners[n_c].x
                  << ", " << bins[n_bin].corners[n_c].y << ") ";
        // Test without rotation
        std::cout << "NR(" << bins[n_bin].getScore(n_c, box, false) << ") ";
        if (temp_score = bins[n_bin].getScore(n_c, box, false);
            temp_score > score) {
          best_bin = n_bin;
          best_corner = n_c;
          score = temp_score;
          best_rotated = false;
        }
        // Test with rotation
        std::cout << "R(" << bins[n_bin].getScore(n_c, box, true) << ") "
                  << std::endl;
        if (temp_score = bins[n_bin].getScore(n_c, box, true);
            temp_score > score) {
          best_bin = n_bin;
          best_corner = n_c;
          score = temp_score;
          best_rotated = true;
        }
      }
    }

    // Put it in the box
    if (score > 0) {
      std::cout << "Put the box in " << best_bin << " at " << best_corner
                << ((best_rotated) ? " [Rotated]" : " [Not Rotated]")
                << " with score " << score << std::endl;
      bins[best_bin].putIn(best_corner, box, best_rotated);
    } else {
      std::cout << "Put in a new box at " << best_corner
                << ((best_rotated) ? " [Rotated]" : " [Not Rotated]")
                << " with score " << score << std::endl;
      bins.push_back(bin::Bin<ILinkedMesh>(format));
      bins[bins.size() - 1].putIn(0, box, false);
    }
    std::cout << std::endl;
  }
  return bins;
}

std::string LinkedMeshPool::getAsSVGString(bin::Bin<ILinkedMesh> &bin,
                                           int max_depth, double sf) const {
  std::stringstream ss;
  ss << "<svg width=\"" << sf * bin.format.width << "\"";
  ss << " height=\"" << sf * bin.format.height << "\"";
  ss << " xmlns=\"http://www.w3.org/2000/svg\">\n";
  for (auto &box : bin.boxes) {
    // Get translation + scaling transformation matrix
    math::HMat mat;
    std::cout << "\tExporting box (" << box.x << ", " << box.y << ", "
              << box.getWidth() << ", " << box.getHeight() << ")" << std::endl;

    // Rotation part
    mat(0, 0) = sf * ((box.rotated) ? 0 : 1);
    mat(0, 1) = sf * ((box.rotated) ? 1 : 0);
    mat(1, 0) = sf * ((box.rotated) ? -1 : 0);
    mat(1, 1) = sf * ((box.rotated) ? 0 : 1);
    mat(2, 2) = sf;

    // Translation part
    mat(0, 3) = sf * box.x;
    mat(1, 3) = sf * box.y;

    std::cout << mat << std::endl;

    auto b = box.root->getBounds(true, true);
    std::cout << "\tHas bound (" << b.xmin << ", " << b.xmax << ", " << b.ymin
              << ", " << b.ymax << ")" << std::endl;

    box.root->fillSVGString(ss, mat, 0, max_depth);
  }
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