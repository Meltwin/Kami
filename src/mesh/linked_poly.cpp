#include "kami/mesh/linked_poly.hpp"
#include "kami/export/line_settings.hpp"
#include "kami/global/arguments.hpp"
#include "kami/math/base_types.hpp"
#include "kami/math/edge.hpp"
#include "kami/math/vertex.hpp"
#include "kami/mesh/linked_edge.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

namespace kami {

// ==========================================================================
// Getters
// ==========================================================================

const math::Bounds LinkedPolygon::getBounds(bool recursive,
                                            bool stop_on_cut) const {
  math::Bounds b;
  for (int i = 0; i < n_edges; i++)
    b += facets[i].getBounds();

  if (recursive) {
    for (int i = 0; i < n_edges; i++) {
      if ((facets[i].isOwned()) && (!stop_on_cut || !facets[i].hasCut()))
        b += facets[i].getMesh()->getBounds(recursive, stop_on_cut);
    }
  }
  return b;
};

const void LinkedPolygon::getBarycenter(math::Barycenter &bary, bool recursive,
                                        bool stop_on_cut) const {
  for (int i = 0; i < n_edges; i++) {
    bary.addVertex(facets[i].getFirst());
    if (recursive && facets[i].isOwned() &&
        (!stop_on_cut || !facets[i].hasCut()))
      facets[i].getMesh()->getBarycenter(bary, recursive, stop_on_cut);
  }
};

// ==========================================================================
// Transformations
// ==========================================================================

math::HMat LinkedPolygon::getHRotationMatrix() const {
  math::HMat mat;

  // Constructing parent frame
  math::Vec3 x_axis =
      getEdgeDirection(parent_edge, true); // Edge direction == new X axis
  math::Vec3 new_n = getParentNormal(); // Parent normal direction == new Z axis
  math::Vec3 y_axis =
      new_n.cross(x_axis); // Y direction is the cross product of the other two
  y_axis.normalize();

  math::Vec3 old_n = getNormal(); // Child normal direction (old Z axis)

  double theta = M_PI / 2 - std::atan2(new_n.dot(old_n), y_axis.dot(old_n));

  // Set the matrix for a rotation around X
  mat(1, 1) = std::cos(theta);
  mat(1, 2) = -std::sin(theta);
  mat(2, 1) = std::sin(theta);
  mat(2, 2) = std::cos(theta);

  mat.simplify();

  return mat;
}

math::HMat LinkedPolygon::getHTransform(int edge) const {
  math::HMat mat;

  // Edge direction == new X axis
  math::Vec3 x_axis = getEdgeDirection(parent_edge, true);
  mat.setRotXAsAxis(x_axis);

  // Parent normal direction == new Z axis
  math::Vec3 z_axis = getParentNormal();
  mat.setRotZAsAxis(z_axis);

  // Y direction is the cross product of the other two
  mat.setRotYAsAxis(z_axis.cross(x_axis));

  // Translation part
  mat.setTransAsAxis(getEdgePosition(parent_edge));

  return mat;
}

void LinkedPolygon::transform(const math::HMat &mat, bool recusive,
                              bool stop_on_cut) {

  // Transform facets
  for (int i = 0; i < n_edges; i++)
    facets[i].transformEdge(mat);

  // Transmit the transformation to the children
  if (recusive) {
    for (int i = 0; i < n_edges; i++) {
      if (facets[i].isOwned() && (!stop_on_cut || !facets[i].hasCut()))
        facets[i].getMesh()->transform(mat, recusive, stop_on_cut);
    }
  }
}

void LinkedPolygon::unfoldMesh(long depth, long max_depth) {
  // If max depth, stop
  if (max_depth != args::NO_REC_LIMIT && depth >= max_depth)
    return;

  // Get the cumulated transformation
  auto rot_mat = getHRotationMatrix();
  auto trsf_mat = getHTransform(parent_edge);
  auto inv_trsf_mat = trsf_mat.invert();

  unfold_coef =
      (math::Mat4)(getParentTrsf() * trsf_mat * rot_mat * inv_trsf_mat);

  // Rotate this face and normal
  transform(unfold_coef, false, false);

  // Rotate children
  for (int i = 0; i < n_edges; i++) {
    if (facets[i].isOwned())
      facets[i].getMesh()->unfoldMesh(depth + 1, max_depth);
  }

  // Change Normal
  math::Vec4 result = (math::Vec4)(unfold_coef * n);
  n = math::Vertex(result(0), result(1), result(2), 0);
  n.simplify();
  n.normalize();
}

// ==========================================================================
// Linking logic
// ==========================================================================

std::vector<ulong> LinkedPolygon::linkNeighbours(LinkedPool &pool) {
  unsigned int done = 0;
  unsigned int max = 0;
  for (int i = 0; i < n_edges; i++) {
    done |= ((facets[i].nullMesh()) ? 0 : (int)(std::pow(2, i)));
    max |= (unsigned int)std::pow(2, i);
  }

  std::vector<ulong> created(0);
  for (ulong i = 0; i < pool.size(); i++) {
    if (uid == pool[i]->uid)
      continue;

    bool unlinked_facet = (pool[i]->parent_edge == INT8_MAX);
    for (int f = 0; f < n_edges; f++) {
      if (facets[f].nullMesh() && pool[i]->hasSameEdge(this, f)) {
        facets[f].linkEdgeAsOwner(pool[i].get(), unlinked_facet);
        if (unlinked_facet)
          created.push_back(i);
        done |= (unsigned int)std::pow(2, f);
        break;
      }
    }

    if (done == max)
      break;
  }

  return created;
}

bool LinkedPolygon::hasSameEdge(LinkedPolygon *parent_facet, int edge) {
  // Get the vertex we want to find
  auto pair = parent_facet->getEdgeVertex(edge);

  for (int i = 0; i < n_edges; i++) {
    if (facets[i].sameAs(pair)) {
      facets[i].setMesh(parent_facet);

      // Linking edge number
      parent_facet->getEdge(edge).setOtherEdge(i);
      facets[i].setOtherEdge(edge);

      // Change ownership
      if (parent_edge == INT8_MAX) {
        facets[i].setLineStyle(LineStyle::INNER);
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

void LinkedPolygon::sliceEdge(int edge) {
  // Cut the edge on this side
  facets[edge].setCutted(true);
  facets[edge].getMesh()->cutOnParentEdge(facets[edge].getCutNumber());

  // Move the child to the center
  auto b = facets[edge].getMesh()->getBounds(true, true);
  math::HMat mat;
  mat.setTransAsAxis(math::Vec3{-b.xmin, -b.ymin, 0});
  facets[edge].getMesh()->transform(mat, true, true);
}

void LinkedPolygon::cutOnParentEdge(int cut_number) {
  facets[parent_edge].setCutted(true, cut_number);
}

overlaps::MeshOverlaps LinkedPolygon::hasOverlaps(const LinkedPool &pool) {
  overlaps::MeshOverlaps out;

  for (const auto &mesh : pool) {
    // If same mesh, skip
    if (mesh->uid == uid)
      continue;

    bool found = false;
    for (int th = 0; th < facets.size(); th++) {
      for (int oth = 0; oth < mesh->facets.size(); oth++) {
        auto params = LinkedEdge<LinkedPolygon>::findIntersect(
            facets[th], mesh->facets[oth]);

        if ((params.t >= math::Edge::VERTEX_AREA) &&
            (params.t <= 1 - math::Edge::VERTEX_AREA) &&
            (params.s >= math::Edge::VERTEX_AREA) &&
            (params.s <= 1 - math::Edge::VERTEX_AREA)) {
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
LinkedPolygon::sliceChildren(const LinkedPool &pool,
                             std::vector<packing::Box<LinkedPolygon>> &boxes) {
  // Populate overlaps
  std::vector<overlaps::MeshOverlaps> overlaps(n_edges + 1);
  for (int i = 0; i < n_edges; i++) {
    overlaps[i] = (facets[i].isOwned() && !facets[i].nullMesh())
                      ? facets[i].getMesh()->sliceChildren(pool, boxes)
                      : overlaps::MeshOverlaps();
  }
  overlaps[n_edges] = hasOverlaps(pool);

  // TODO: Remove, it's debug
  std::cout << "Overlaps for " << uid << std::endl;
  for (int i = 0; i < n_edges; i++) {
    std::cout << "\t - " << getEdgeName(i) << " has " << overlaps[i].size()
              << " overlaps" << std::endl;
  }
  std::cout << "\t - It has " << overlaps[n_edges].size() << " overlaps"
            << std::endl;

  // Check the edges against the others
  for (int i = 0; i < n_edges; i++) {

    // Compute the intersection of the overlapping vector
    overlaps::MeshOverlaps intersection;
    for (int j = i + 1; j <= n_edges; j++) {
      intersection = intersection + overlaps[i] / overlaps[j];
    }

    // If the intersection is not null, cut it
    if (intersection.size() > 0) {
      sliceEdge(i);
      boxes.push_back(packing::Box<LinkedPolygon>{
          facets[i].getMesh(),
          facets[i].getMesh()->getBounds(true, true),
      });
      for (int k = 0; k < n_edges; k++) {
        overlaps[k] = overlaps[k] - overlaps[i];
      }
    }
  }

  // Recontruct the new overlapping vector (sum of all the overlapings without
  // the ones processed by this node)
  overlaps::MeshOverlaps new_overlaps;
  for (int i = 0; i <= n_edges; i++) {
    new_overlaps = new_overlaps + overlaps[i];
  }

  return new_overlaps;
}

// ==========================================================================
// Fixations
// ==========================================================================

LinkedPolygon::EdgeInfos LinkedPolygon::getEdgeInfoFixations(
    const LinkedEdge<LinkedPolygon> &edge) const {
  EdgeInfos infos;

  // If not owned, return empty result
  if (!edge.isOwned())
    return infos;

  auto other = edge.getMesh()->getParentEdge();

  infos.u = edge.dir();
  infos.v = other.dir();

  // Get directions
  if ((math::Vertex::distance2(edge.getFirst(), other.getFirst()) <
       math::MAX_DISTANCE2) ||
      (math::Vertex::distance2(edge.getFirst(), other.getSecond()) <
       math::MAX_DISTANCE2)) {
    infos.P = edge.getFirst();
    infos.has_common_point = true;

  } else if ((math::Vertex::distance2(edge.getSecond(), other.getSecond()) <
              math::MAX_DISTANCE2) ||
             (math::Vertex::distance2(edge.getSecond(), other.getFirst()) <
              math::MAX_DISTANCE2)) {
    infos.P = edge.getSecond();
    infos.has_common_point = true;
  }

  // Get angle
  double norm = infos.u.norm() * infos.v.norm();
  double cos = (infos.u(0) * infos.v(0) + infos.u(1) * infos.v(1)) / norm;
  double sin = (infos.u(1) * infos.v(0) + infos.u(0) * infos.v(1)) / norm;
  infos.angle = std::atan2(sin, cos);

  return infos;
}

void LinkedPolygon::makeGlueFixations(const fix::FixationParameters &params,
                                      ulong caller_edge) {
  auto &caller = getEdge(caller_edge);
  auto &other = getOtherEdge(caller_edge);

  // Make P1 (center of segment)
  caller.addFixPoint(caller.lin_interpolation(0));
  other.addFixPoint(other.lin_interpolation(0));
}

void LinkedPolygon::makeClipFixations(const fix::FixationParameters &params,
                                      ulong caller_edge) {
  auto &caller = getEdge(caller_edge);
  auto &other = getOtherEdge(caller_edge);

  // Make P1 (center of segment)
  caller.addFixPoint(caller.lin_interpolation(0.5));
  other.addFixPoint(other.lin_interpolation(0.3));

  // Make P2 (return to the limit for the clip)
  caller.addFixPoint(caller.lin_interpolation(params.clip_limits));
  // other.addFixPoint(other.lin_interpolation(1 - params.clip_limits));

  // Make Y vector for both edges
  auto y_caller = getEdgeYAxis(caller_edge);
  auto y_other = caller.getMesh()->getEdgeYAxis(caller.getOtherEdge());

  // Make P3 (first point out)
  caller.addFixPoint((math::Vec4)(
      caller.lin_interpolation(params.clip_limits +
                               params.height / caller.dir().norm()) +
      params.height * y_caller));
  /*other.addFixPoint((math::Vec4)(
      other.lin_interpolation(1 - params.clip_limits - params.height) +
      params.height * other.dir().norm() * y_other));*/

  // Make P4 (second point out)
  caller.addFixPoint((math::Vec4)(
      caller.lin_interpolation(1 - params.clip_limits -
                               params.height / caller.dir().norm()) +
      params.height * y_caller));
  /*other.addFixPoint(
      (math::Vec4)(other.lin_interpolation(params.clip_limits + params.height) +
                   params.height * other.dir().norm() * y_other));*/

  // Make P5
  caller.addFixPoint(caller.lin_interpolation(1 - params.clip_limits));
  // other.addFixPoint(other.lin_interpolation(params.clip_limits));
}

void LinkedPolygon::makeFixations(const fix::FixationParameters &params) {
  // Call recursively
  for (ulong i = 0; i < facets.size(); i++) {

    // Make the fixation
    if (!facets[i].isOwned() && !facets[i].hasFixations() &&
        !facets[i].isInner()) {
      auto infos = getEdgeInfoFixations(facets[i]);

      if (!facets[i].hasCut() && infos.has_common_point &&
          (infos.angle <= M_PI_2))
        makeGlueFixations(params, i);
      else
        makeClipFixations(params, i);
    }

    if (facets[i].isOwned()) {
      // Recursive call on the children
      facets[i].getMesh()->makeFixations(params);
    }
  }
}

// ==========================================================================
// SVG Export
// ==========================================================================

void LinkedPolygon::fillSVGString(std::stringstream &stream,
                                  const math::HMat &mat, const args::Args &args,
                                  int depth) {
  if (args.max_depth != -1 && depth >= args.max_depth)
    return;

  transform(mat, false, true);

  // Draw this facet
  for (int i = 0; i < n_edges; i++) {
    facets[i].setTextRatio(mat(2, 2));
    facets[i].getAsSVGLine(stream, args);
  }

  // Call the children
  for (int i = 0; i < n_edges; i++) {
    if (facets[i].isOwned() && !facets[i].hasCut()) {
      facets[i].getMesh()->fillSVGString(stream, mat, args, depth + 1);
    }
  }
};

} // namespace kami
