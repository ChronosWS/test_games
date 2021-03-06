#pragma once

#include "vec.h"
#include "rect.h"

namespace math
{
// Does 2d line segment intersection.
bool LineSegmentsIntersect(const math::Vec2f& a_start, const math::Vec2f& a_end,
                           const math::Vec2f& b_start, const math::Vec2f& b_end,
                           float* time, math::Vec2f* position);

bool PointInPolygon(const math::Vec2f& point, const uint64_t polygon_size,
                    math::Vec2f* polygon);

bool PointInRect(const math::Vec2f& point, const AxisAlignedRect& rect);

}  // namespace math
