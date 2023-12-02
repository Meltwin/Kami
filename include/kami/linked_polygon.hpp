#ifndef KAMI_LINKED_POLYGON
#define KAMI_LINKED_POLYGON

#include "kami/bin_packing.hpp"
#include "kami/display_settings.hpp"
#include "kami/linked_edge.hpp"
#include "kami/linked_mesh.hpp"
#include "kami/math.hpp"
#include "kami/overlaps.hpp"
#include <cstdint>
#include <memory>

namespace kami {

constexpr double MIN_DIST{1E-2};

/**
 * @brief Represent a facet and its link with the neighbours
 */

template <int N> struct LinkedMesh : public ILinkedMesh {

  // ==========================================================================
  // Facet description
  // ==========================================================================
  // Facet properties
  std::vector<LinkedEdge<ILinkedMesh>> facets =
      std::vector<LinkedEdge<ILinkedMesh>>(N); //< Edges of this facet

  LinkedMesh() : ILinkedMesh() {}

  // ==========================================================================
  // Getters
  // ==========================================================================

  LinkedEdge<ILinkedMesh> getEdge(int edge) const override {
    if (edge < N)
      return facets[edge];
    return facets[0];
  }

  std::string getEdgeName(int edge) const override {
    std::stringstream ss;
    ss << "f" << edge + 1 << ((edge == N - 1) ? 1 : edge + 2);
    return ss.str();
  }

  const ILinkedMesh *getParent() const override {
    if (parent_edge < N)
      return facets[parent_edge].mesh;
    return nullptr;
  }

  const Bounds getBounds(bool recursive,
                         bool stop_on_cut = true) const override {
    Bounds b;
    for (int i = 0; i < N; i++)
      b += facets[i].getBounds();

    if (recursive) {
      for (int i = 0; i < N; i++) {
        if ((facets[i].owned) && (!stop_on_cut || !facets[i].cutted))
          b += facets[i].mesh->getBounds(recursive, stop_on_cut);
      }
    }
    return b;
  };

  const void getBarycenter(math::Barycenter &bary, bool recursive,
                           bool stop_on_cut = true) const override {
    for (int i = 0; i < N; i++) {
      bary.addVertex(facets[i].v1);
      if (recursive && facets[i].owned && (!stop_on_cut || !facets[i].cutted))
        facets[i].mesh->getBarycenter(bary, recursive, stop_on_cut);
    }
  };

  // ==========================================================================
  // Transformations
  // ==========================================================================

  void transform(const math::HMat &mat, bool recusive = false,
                 bool stop_on_cut = true) override {

    // Transform facets
    for (int i = 0; i < N; i++) {
      facets[i].v1 = (Vec4)(mat * facets[i].v1);
      facets[i].v2 = (Vec4)(mat * facets[i].v2);
    }

    // Transmit the transformation to the children
    if (recusive) {
      for (int i = 0; i < N; i++) {
        if (facets[i].owned && (!stop_on_cut || !facets[i].cutted))
          facets[i].mesh->transform(mat, recusive, stop_on_cut);
      }
    }
  }

  void unfoldMesh(long depth, long max_depth) override {
    // If max depth, stop
    if (max_depth != NO_REC_LIMIT && depth >= max_depth)
      return;

    // Get the cumulated transformation
    auto rot_mat = getHRotationMatrix();
    auto trsf_mat = getHTransform(parent_edge);
    auto inv_trsf_mat = trsf_mat.invert();

    unfold_coef = (Mat4)(getParentTrsf() * trsf_mat * rot_mat * inv_trsf_mat);

    // Rotate this face and normal
    transform(unfold_coef, false, false);

    // Rotate children
    for (int i = 0; i < N; i++) {
      if (facets[i].owned)
        facets[i].mesh->unfoldMesh(depth + 1, max_depth);
    }

    // Change Normal
    Vec4 result = (Vec4)(unfold_coef * n);
    n = math::Vertex(result(0), result(1), result(2), 0);
    n.simplify();
    n.normalize();
  }

  // ==========================================================================
  // Linking logic
  // ==========================================================================

  std::vector<ulong> linkNeighbours(LinkedPool &pool) override {
    unsigned int done = 0;
    unsigned int max = 0;
    for (int i = 0; i < N; i++) {
      done |= ((facets[i].mesh == nullptr) ? 0 : (int)(std::pow(2, i)));
      max |= (unsigned int)std::pow(2, i);
    }

    std::vector<ulong> created(0);
    for (ulong i = 0; i < pool.size(); i++) {
      if (uid == pool[i]->uid)
        continue;

      bool unlinked_facet = (pool[i]->parent_edge == INT8_MAX);
      for (int f = 0; f < N; f++) {
        if ((facets[f].mesh == nullptr) && pool[i]->hasSameEdge(this, f)) {
          if (unlinked_facet) {
            facets[f].owned = true;
            created.push_back(i);
            facets[f].linestyle = SVGLineWidth::INNER;
          }
          facets[f].mesh = pool[i].get();
          done |= (unsigned int)std::pow(2, f);
          break;
        }
      }

      if (done == max)
        break;
    }

    return created;
  }

  bool hasSameEdge(ILinkedMesh *parent_facet, int edge) override {
    // Get the vertex we want to find
    auto pair = parent_facet->getEdgeVertex(edge);

    for (int i = 0; i < N; i++) {
      if (facets[i].sameAs(pair)) {
        facets[i].mesh = parent_facet;
        if (parent_edge == INT8_MAX) {
          facets[i].linestyle = SVGLineWidth::INNER;
          parent_edge = (parent_edge == INT8_MAX) ? i : parent_edge;
        }
        return true;
      }
    }

    return false;
  }

  // ==========================================================================
  // Sclicing logic
  // ==========================================================================

  void sliceEdge(int edge) override {
    // Cut the edge on this side
    facets[edge].linestyle = SVGLineWidth::CUTTED;
    facets[edge].cutted = true;
    facets[edge].cut_number = LinkedEdge<ILinkedMesh>::getCutNumber();
    facets[edge].mesh->cutOnParentEdge(facets[edge].cut_number);

    // Move the child to the center
    auto b = facets[edge].mesh->getBounds(true, true);
    math::HMat mat;
    mat.setTransAsAxis(Vec3{-b.xmin, -b.ymin, 0});
    facets[edge].mesh->transform(mat, true, true);
  }

  void cutOnParentEdge(int cut_number) override {
    facets[parent_edge].cutted = true;
    facets[parent_edge].linestyle = SVGLineWidth::CUTTED;
    facets[parent_edge].cut_number = cut_number;
  }

  overlaps::MeshOverlaps hasOverlaps(const LinkedPool &pool) override {
    overlaps::MeshOverlaps out;

    for (const auto &mesh : pool) {
      // If same mesh, skip
      if (mesh->uid == uid)
        continue;

      auto casted_elem = static_cast<LinkedMesh *>(mesh.get());
      bool found = false;
      for (int th = 0; th < facets.size(); th++) {
        for (int oth = 0; oth < casted_elem->facets.size(); oth++) {
          auto params = LinkedEdge<ILinkedMesh>::findIntersect(
              facets[th], casted_elem->facets[oth]);

          if ((params.first >= MIN_DIST) && (params.first <= 1 - MIN_DIST) &&
              (params.second >= MIN_DIST) && (params.second <= 1 - MIN_DIST)) {
            found = true;
            out.push_back(overlaps::Overlap{uid, mesh->uid});
            break;
          }
        }
        if (found)
          break;
      }
    }

    return out;
  }

  overlaps::MeshOverlaps
  sliceChildren(const LinkedPool &pool,
                std::vector<bin::Box<ILinkedMesh>> &boxes) override {
    // Populate overlaps
    std::vector<overlaps::MeshOverlaps> overlaps(N + 1);
    for (int i = 0; i < N; i++) {
      overlaps[i] = (facets[i].owned && facets[i].mesh != nullptr)
                        ? facets[i].mesh->sliceChildren(pool, boxes)
                        : overlaps::MeshOverlaps();
    }
    overlaps[N] = hasOverlaps(pool);

    // TODO: Remove, it's debug
    std::cout << "Overlaps for " << uid << std::endl;
    for (int i = 0; i < N; i++) {
      std::cout << "\t - " << getEdgeName(i) << " has " << overlaps[i].size()
                << " overlaps" << std::endl;
    }
    std::cout << "\t - It has " << overlaps[N].size() << " overlaps"
              << std::endl;

    // Check the edges against the others
    for (int i = 0; i < N; i++) {

      // Compute the intersection of the overlapping vector
      overlaps::MeshOverlaps intersection;
      for (int j = i + 1; j <= N; j++) {
        intersection = intersection + overlaps[i] / overlaps[j];
      }

      // If the intersection is not null, cut it
      if (intersection.size() > 0) {
        sliceEdge(i);
        boxes.push_back(bin::Box<ILinkedMesh>{
            facets[i].mesh,
            facets[i].mesh->getBounds(true, true),
        });
        for (int k = 0; k < N; k++) {
          overlaps[k] = overlaps[k] - overlaps[i];
        }
      }
    }

    // Recontruct the new overlapping vector (sum of all the overlapings without
    // the ones processed by this node)
    overlaps::MeshOverlaps new_overlaps;
    for (int i = 0; i <= N; i++) {
      new_overlaps = new_overlaps + overlaps[i];
    }

    return new_overlaps;
  }

  // ==========================================================================
  // STL Model Unfold + SVG Export
  // ==========================================================================

  void fillSVGString(std::stringstream &stream, const math::HMat &mat,
                     int depth, int max_depth) override {
    if (max_depth != -1 && depth >= max_depth)
      return;

    transform(mat, false, true);

    // Draw this facet
    for (int i = 0; i < N; i++) {
      facets[i].text_size *= mat(2, 2);
      facets[i].getAsSVGLine(stream);
    }

    // Call the children
    for (int i = 0; i < N; i++) {
      if (facets[i].owned && !facets[i].cutted) {
        facets[i].mesh->fillSVGString(stream, mat, depth + 1, max_depth);
      }
    }
  };

  // ==========================================================================
  // Debug
  // ==========================================================================
  void displayInformations(std::ostream &os) const override {
    os << "Mesh " << uid << " : " << std::endl;
    for (int i = 0; i < N; i++) {
      os << "  - f" << i + 1 << ((i == N - 1) ? 1 : i + 2) << " " << facets[i]
         << std::endl;
    }
  }
};

struct LinkedTriangle : public LinkedMesh<3> {
  LinkedTriangle();
  LinkedTriangle(microstl::Facet *facet, ulong _id);
};

} // namespace kami

#endif