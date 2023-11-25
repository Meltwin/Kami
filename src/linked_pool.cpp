#include "kami/linked_pool.hpp"
#include "kami/arguments.hpp"
#include "kami/bin_packing.hpp"
#include "kami/bounds.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/linked_polygon.hpp"
#include <cmath>
#include <sstream>
#include <vector>

namespace kami {

// ==========================================================================
// Constructor
// ==========================================================================

LinkedMeshPool::LinkedMeshPool(microstl::Mesh &mesh)
    : std::vector<std::shared_ptr<ILinkedMesh>>(mesh.facets.size()) {
  for (ulong i = 0; i < mesh.facets.size(); i++) {
    (*this)[i] = std::make_shared<LinkedTriangle>(&mesh.facets[i], i);
  }
}

// ==========================================================================
// Linking
// ==========================================================================

void LinkedMeshPool::makeFacetPoolInternalLink() {
  ulong index = 0;
  std::vector<ulong> stack{0};
  while ((index < stack.size()) && (index < this->size())) {
    auto created = (*this)[stack[index]]->linkNeighbours(*this);
    stack.insert(stack.end(), created.begin(), created.end());
    index++;
  }
}

// ==========================================================================
// Slicing
// ==========================================================================

MeshBinVector LinkedMeshPool::slice() {
  MeshBoxVector boxes;
  (*this)[root]->sliceChildren(*this, boxes);

  // Transforming the root
  auto b = (*this)[root]->getBounds(true, true);
  math::HMat mat;
  mat.setTransAsAxis(Vec3{-b.xmin, -b.ymin, 0});
  (*this)[root]->transform(mat, true, true);

  // Adding the root to the list
  boxes.push_back(MeshBox((*this)[root].get(), (*this)[root]->getBounds(true)));

  // Debug
  std::cout << "Got " << boxes.size() << " parts for this mesh" << std::endl;
  for (auto &b : boxes) {
    std::cout << "\t" << b << std::endl;
  }

  // Launch the bin packing
  return binPackingAlgorithm(boxes);
}

MeshBinVector LinkedMeshPool::binPackingAlgorithm(MeshBoxVector &boxes) {
  // Sorting the items by decreasing value
  std::sort(boxes.begin(), boxes.end(), [](MeshBox &elem1, MeshBox &elem2) {
    return (elem1.height * elem1.width) > (elem2.width * elem2.height);
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

  std::cout << "Using bin format " << format << std::endl;

  // PHASE 1: Compute the lower bound (number of bins to open)
  double L = 0;
  for (auto &box : boxes) {
    L += box.height * box.width;
  }
  int L0 = std::ceil(L / (format.height * format.width));
  MeshBinVector bins(L0, format);

  // PHASE 2: Packing the boxes
  for (auto &box : boxes) {
    double score = 0, temp_score = 0;

    std::cout << "Packing " << box << std::endl;

    // Compute best position
    ulong best_bin = 0;
    ulong best_corner = 0;
    ulong best_rotated = false;
    for (ulong n_bin = 0; n_bin < bins.size(); n_bin++) {
      std::cout << "\tBin " << n_bin + 1 << " corners :" << std::endl;
      for (ulong n_c = 0; n_c < bins[n_bin].corners.size(); n_c++) {
        std::cout << "\t\t" << n_c << " -> " << bins[n_bin].corners[n_c];

        // Test without rotation
        std::cout << " NR(" << bins[n_bin].getScore(n_c, box, false) << ") ";
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
      std::cout << "\tPut the box in " << best_bin << " at " << best_corner
                << ((best_rotated) ? " [Rotated]" : " [Not Rotated]")
                << " with score " << score << std::endl;
      bins[best_bin].putIn(best_corner, box, best_rotated);
    } else {
      std::cout << "\tPut in a new box at " << best_corner
                << ((best_rotated) ? " [Rotated]" : " [Not Rotated]")
                << " with score " << score << std::endl;
      bins.push_back(bin::Bin<ILinkedMesh>(format));
      bins[bins.size() - 1].putIn(0, box, false);
    }
    std::cout << std::endl;
  }
  return bins;
}

// ==========================================================================
// Exporting
// ==========================================================================

std::string LinkedMeshPool::getAsSVGString(MeshBin &bin,
                                           const args::Args &args) const {
  std::stringstream ss;
  ss << "<svg width=\"" << args.resolution * bin.format.width << "\"";
  ss << " height=\"" << args.resolution * bin.format.height << "\"";
  ss << " xmlns=\"http://www.w3.org/2000/svg\">\n";
  for (auto &box : bin.boxes) {
    // Get translation + scaling transformation matrix
    math::HMat mat;
    std::cout << "\t\tExporting " << box;

    // Rotation part
    mat(0, 0) = args.resolution * ((box.rotated) ? 0 : 1);
    mat(0, 1) = args.resolution * ((box.rotated) ? 1 : 0);
    mat(1, 0) = args.resolution * ((box.rotated) ? -1 : 0);
    mat(1, 1) = args.resolution * ((box.rotated) ? 0 : 1);
    mat(2, 2) = args.resolution;

    // Translation part
    mat(0, 3) = args.resolution * box.x;
    mat(1, 3) = args.resolution * box.y;

    auto b = box.root->getBounds(true, true);
    std::cout << " of " << b << std::endl;

    box.root->fillSVGString(ss, mat, 0, args.max_depth);

    if (args.svg_debug)
      ss << "<rect x=\"" << args.resolution * box.x << "\" y=\""
         << args.resolution * box.y << "\" width=\""
         << args.resolution * box.getWidth() << "\" height=\""
         << args.resolution * box.getHeight()
         << "\" style=\"fill:transparent;stroke:red;stroke-width:5;\" />\n";
  }

  // If in svg debug, add the corners of the bin
  if (args.svg_debug) {
    for (auto &c : bin.corners) {
      ss << "<circle cx=\"" << args.resolution * c.x << "\" cy=\""
         << args.resolution * c.y << "\" r=\"" << 2 * args.resolution
         << "\" stroke=\"" << c.getStrokeColor()
         << "\" stroke-width=\"3\" fill=\"" << c.getFillColor()
         << "\" fill-opacity=\"0.5\" stroke-opacity=\"0.5\"/>\n";
    }
  }
  ss << "</svg>";
  return ss.str();
}

// ==========================================================================
// Debug
// ==========================================================================s

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