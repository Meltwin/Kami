#ifndef KAMI_LINKED_PLAN
#define KAMI_LINKED_PLAN

#include "kami/math.hpp"
#include <vector>

namespace kami {

/**
 * @brief Segment for the planar pattern of the mesh
 *
 */
struct PlanSegment : Vec4 {};

/**
 * @brief Facet in the plan
 *
 */
struct LinkedPlanFacet : std::vector<PlanSegment> {};

/**
 * @brief Container the planar pattern of the mesh
 *
 */
struct LinkedPlanPool : std::vector<LinkedPlanFacet> {};

} // namespace kami

#endif