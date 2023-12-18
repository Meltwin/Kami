#include "kami/mesh/linked_pool.hpp"
#include "kami/export/color.hpp"
#include "kami/global/arguments.hpp"
#include "kami/global/logging.hpp"
#include "kami/math/barycenter.hpp"
#include "kami/math/bounds.hpp"
#include "kami/math/hmat.hpp"
#include "kami/mesh/linked_implementations.hpp"
#include "kami/mesh/linked_poly.hpp"
#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace kami {

// ==========================================================================
// Constructor
// ==========================================================================

LinkedMeshPool::LinkedMeshPool(microstl::Mesh &mesh)
    : std::vector<std::shared_ptr<LinkedPolygon>>(mesh.facets.size()) {
  for (ulong i = 0; i < mesh.facets.size(); i++) {
    (*this)[i] = std::make_shared<LinkedTriangle>(&mesh.facets[i], i);
  }
  this->makeFacetPoolInternalLink();
}

// ==========================================================================
// Linking
// ==========================================================================

void LinkedMeshPool::makeFacetPoolInternalLink() {
  TIMED_UTILS;
  TIMED_SECTION("Mesh preparation", {
    // Merging facets of same normal
    printStepHeader("Merging facets of same direction");
    std::vector<ulong> removed(0);
    for (auto &poly : *this) {
      if (auto it = std::find_if(
              removed.begin(), removed.end(),
              [&poly](ulong &other) { return other == poly->getUID(); });
          it == removed.end()) {
        poly->mergeSimilar(*this, removed);
      }
    }

    // Remove all faces that have been merged from the pool
    printStepHeader("Removing merged faces");
    for (const auto &uid : removed) {
      if (auto it = std::find_if(this->begin(), this->end(),
                                 [&uid](std::shared_ptr<LinkedPolygon> &poly) {
                                   return poly->getUID() == uid;
                                 });
          it != this->end()) {
        this->erase(it);
      }
    }

    // Backuping everything
    for (auto &f : *this) {
      _unfold_unlinked.push_back(*f.get());
    }

    // Linking every facet
    printStepHeader("Mesh Linking");
    ulong index = 0;
    std::vector<ulong> stack{0};
    while ((index < stack.size()) && (index < this->size())) {
      auto created = (*this)[stack[index]]->linkNeighbours(*this);
      stack.insert(stack.end(), created.begin(), created.end());
      index++;
    }

    _unfolded_bounds += (*this)[root]->getBounds(true);
  })
}

// ==========================================================================
// Slicing
// ==========================================================================

MeshBinVector LinkedMeshPool::slice() {
  MeshBoxVector boxes;

  TIMED_UTILS;
  TIMED_SECTION("Mesh slicing", {
    (*this)[root]->sliceChildren(*this, boxes);

    // Transforming the root
    auto b = (*this)[root]->getBounds(true, true);
    math::HMat mat;
    mat.setTransAsAxis(math::Vec3{-b.xmin, -b.ymin, 0});
    (*this)[root]->transform(mat, true, true);

    // Adding the root to the list
    boxes.push_back(
        MeshBox((*this)[root].get(), (*this)[root]->getBounds(true)));

    printStepHeader("Slicing result");
    std::cout << "Got " << boxes.size() << " parts for this mesh" << std::endl;
    for (auto &b : boxes) {
      std::cout << "\t" << b << std::endl;
    }
  });

  TIMED_SECTION("Making boxes colors", color_map = makeColorMap(boxes));

  // Launch the bin packing
  MeshBinVector bins;
  TIMED_SECTION("Paper box packing", bins = binPackingAlgorithm(boxes));
  return bins;
}

MeshBinVector LinkedMeshPool::binPackingAlgorithm(MeshBoxVector &boxes) {
  // Sorting the items by decreasing value
  std::sort(boxes.begin(), boxes.end(), [](MeshBox &elem1, MeshBox &elem2) {
    return (elem1.height * elem1.width) > (elem2.width * elem2.height);
  });
  int id = 0;
  for (auto &b : boxes)
    b.id = id++;

  // Orient them horizontally
  for (auto &box : boxes) {
    if (box.width < box.height) {
      box.rotated = true;
    }
  }

  std::cout << "Using bin format " << format << std::endl;

  // PHASE 1: Compute the lower bound (number of bins to open)
  double L = 0;
  for (auto &box : boxes) {
    L += box.height * box.width;
  }
  int L0 = std::ceil(L / (format.height * format.width));
  MeshBinVector bins(0);
  for (ulong i = 0; i < L0; i++)
    bins.push_back(MeshBin(format));

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
        temp_score = bins[n_bin].getScore(n_c, box, box.rotated);
        std::cout << " NR(" << temp_score << ") ";
        if (temp_score > score) {
          best_bin = n_bin;
          best_corner = n_c;
          score = temp_score;
          best_rotated = box.rotated;
        }

        // Test with rotation
        temp_score = bins[n_bin].getScore(n_c, box, !box.rotated);
        std::cout << "R(" << temp_score << ") " << std::endl;
        if (temp_score > score) {
          best_bin = n_bin;
          best_corner = n_c;
          score = temp_score;
          best_rotated = !box.rotated;
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
      bins.push_back(packing::Bin<LinkedPolygon>(format));
      bins[bins.size() - 1].putIn(0, box, false);
    }
    std::cout << std::endl;
  }
  return bins;
}

// ==========================================================================
// Exporting
// ==========================================================================

std::map<ulong, std::string>
LinkedMeshPool::makeColorMap(const MeshBoxVector &boxes) const {
  std::map<ulong, std::string> color_map;
  color::ColorGenerator gen = color::ColorGenerator::basicGenerator();

  std::vector<ulong> uids;
  for (auto &box : boxes) {
    // Get uids
    uids.resize(0);
    box.root->getChildUIDs(uids);

    auto color = gen.makeNewColor();
    std::cout << color.str() << std::endl;

    // Assign in the map
    for (auto id : uids)
      color_map.insert_or_assign(id, color.str());
  }

  return color_map;
}

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
    // mat(0, 0) = args.resolution;
    // mat(1, 1) = args.resolution;
    mat(2, 2) = args.resolution;

    // Translation part
    mat(0, 3) = args.resolution * box.x;
    mat(1, 3) =
        args.resolution * (box.y + ((box.rotated) ? box.getHeight() : 0));

    auto b = box.root->getBounds(true, true);
    std::cout << " of " << b << std::endl;

    std::cout << mat << std::endl;

    box.root->fillSVGString(ss, mat, color_map.at(box.root->getUID()), 0,
                            args.max_depth);

    if (args.svg_debug) {
      ss << "<rect x=\"" << args.resolution * box.x << "\" y=\""
         << args.resolution * box.y << "\" width=\""
         << args.resolution * box.getWidth() << "\" height=\""
         << args.resolution * box.getHeight()
         << "\" style=\"fill:red;stroke:red;stroke-width:5;fill-opacity:0.3;\" "
            "/>\n";
      ss << "<text x=\"" << args.resolution * (2 * box.x + box.getWidth()) / 2
         << "\" y=\"" << args.resolution * (2 * box.y + box.getHeight()) / 2
         << "\" font-size=\"" << 4 * args.resolution << "px\">" << box.id
         << "</text>";
    }
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
// Projections
// ==========================================================================

struct ProjectionOrder {
  ulong uid;
  double value;
};

std::string LinkedMeshPool::getProjectionAsString(const math::Vec3 &ax1,
                                                  const math::Vec3 &ax2,
                                                  const args::Args &args) {
  auto normal = ax1.cross(ax2);

  // Get the bounds of the figure
  math::Bounds fig_bounds;
  math::Vec3 pt;
  double on1, on2;
  for (char pi = 0; pi < 8; pi++) {
    pt = math::Vec3{
        ((pi & 0x01) == 0x01) ? _unfolded_bounds.xmin : _unfolded_bounds.xmax,
        ((pi & 0x02) == 0x02) ? _unfolded_bounds.ymin : _unfolded_bounds.ymax,
        ((pi & 0x04) == 0x04) ? _unfolded_bounds.zmin : _unfolded_bounds.zmax,
    };
    std::cout << "\tPoint " << +pi << " " << pt(0) << ", " << pt(1) << ", "
              << pt(2) << std::endl;
    on1 = pt.dot(ax1);
    on2 = pt.dot(ax2);
    fig_bounds += math::Bounds{on1, on1, on2, on2, 0, 0};
  }

  // Get the order
  std::vector<ProjectionOrder> order(_unfold_unlinked.size());
  for (ulong i = 0; i < _unfold_unlinked.size(); i++) {
    math::Barycenter bary;
    _unfold_unlinked[i].getBarycenter(bary, false);
    math::Vec3 bary3 = bary.getBarycenter();
    order[i] = ProjectionOrder{_unfold_unlinked[i].getUID(), bary3.dot(normal)};
  }

  // Get the order
  std::sort(order.begin(), order.end(),
            [](ProjectionOrder &p1, ProjectionOrder &p2) {
              return p1.value < p2.value;
            });

  // Get transform matrix
  math::HMat trsf;
  if (!_unfold_transformed) {
    trsf(0, 0) = args.resolution;
    trsf(1, 1) = args.resolution;
    trsf(2, 2) = args.resolution;
    _unfold_transformed = true;
  }

  // Project them
  std::stringstream ss;
  ss << "<svg viewBox=\"";
  ss << args.resolution * fig_bounds.xmin << " "
     << args.resolution * fig_bounds.ymin << " ";
  ss << args.resolution * (fig_bounds.xmax - fig_bounds.xmin) << " "
     << args.resolution * (fig_bounds.ymax - fig_bounds.ymin);
  ss << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
  for (const auto &order_elem : order) {
    _unfold_unlinked[order_elem.uid].fillSVGProjectString(
        ss, trsf, ax1, ax2, color_map.at(order_elem.uid));
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
  printStepHeader("Pool Informations");
  std::cout << "\tSolo facets = " << solo << std::endl;
  std::cout << "\tNon Owning facets = " << non_owning << std::endl;
}

} // namespace kami