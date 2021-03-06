#include "intersection.h"

#include "utils.h"

namespace math
{
float
Signed2DTriArea(const math::Vec2f& a, const math::Vec2f& b,
                const math::Vec2f& c)
{
  return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool
OnSegment(const math::Vec2f& p, const math::Vec2f& q, const math::Vec2f& r)
{
  if (q.x <= Max(p.x, r.x) && q.x >= Min(p.x, r.x) && q.y <= Max(p.y, r.y) &&
      q.y >= Min(p.y, r.y)) {
    return true;
  }
  return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int
Orientation(const math::Vec2f& p, const math::Vec2f& q, const math::Vec2f& r)
{
  float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
  if (IsNear(val, 0.f)) return 0;  // colinear
  return (val > 0.f) ? 1 : 2;      // clock or counterclock wise
}

// The function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool
DoIntersect(const math::Vec2f& p1, const math::Vec2f& q1, const math::Vec2f& p2,
            const math::Vec2f& q2)
{
  // Find the four orientations needed for general and
  // special cases
  int o1 = Orientation(p1, q1, p2);
  int o2 = Orientation(p1, q1, q2);
  int o3 = Orientation(p2, q2, p1);
  int o4 = Orientation(p2, q2, q1);
  // General case
  if (o1 != o2 && o3 != o4) return true;
  // Special Cases
  // p1, q1 and p2 are colinear and p2 lies on segment p1q1
  if (o1 == 0 && OnSegment(p1, p2, q1)) return true;
  // p1, q1 and p2 are colinear and q2 lies on segment p1q1
  if (o2 == 0 && OnSegment(p1, q2, q1)) return true;
  // p2, q2 and p1 are colinear and p1 lies on segment p2q2
  if (o3 == 0 && OnSegment(p2, p1, q2)) return true;
  // p2, q2 and q1 are colinear and q1 lies on segment p2q2
  if (o4 == 0 && OnSegment(p2, q1, q2)) return true;
  return false;  // Doesn't fall in any of the above cases
}

// namespace

// pg 152. Real-Time Collision Detection by Christer Ericson
bool
LineSegmentsIntersect(const math::Vec2f& a_start, const math::Vec2f& a_end,
                      const math::Vec2f& b_start, const math::Vec2f& b_end,
                      float* time, math::Vec2f* position)
{
  float a1 = Signed2DTriArea(a_start, a_end, b_end);
  float a2 = Signed2DTriArea(a_start, a_end, b_start);
  if (a1 * a2 >= 0.f) return false;
  float a3 = Signed2DTriArea(b_start, b_end, a_start);
  float a4 = a3 + a2 - a1;
  if (a3 * a4 >= 0.f) return false;
  if (time && position) {
    *time = a3 / (a3 - a4);
    *position = a_start + (a_end - a_start) * *time;
  }
  return true;
}

// https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
bool
PointInPolygon(const math::Vec2f& point, const uint64_t polygon_size,
               math::Vec2f* polygon)
{
  // There must be at least 3 vertices in polygon[]
  if (polygon_size < 3) return false;
  // Create a point for line segment from p to infinite
  math::Vec2f extreme(std::numeric_limits<float>::max(), point.y);
  // Count intersections of the above line with sides of polygon
  int count = 0, i = 0;
  do {
    int next = (i + 1) % polygon_size;
    // Check if the line segment from 'p' to 'extreme' intersects
    // with the line segment from 'polygon[i]' to 'polygon[next]'
    if (DoIntersect(polygon[i], polygon[next], point, extreme)) {
      // If the point 'p' is colinear with line segment 'i-next',
      // then check if it lies on segment. If it lies, return true,
      // otherwise false
      if (Orientation(polygon[i], point, polygon[next]) == 0) {
        return OnSegment(polygon[i], point, polygon[next]);
      }
      count++;
    }
    i = next;
  } while (i != 0);
  // Return true if count is odd, false otherwise
  return count & 1;  // Same as (count%2 == 1)
}

bool
PointInRect(const math::Vec2f& point, const AxisAlignedRect& rect)
{
  return (point.x > rect.min.x && point.x < rect.max.x) &&
         (point.y > rect.min.y && point.y < rect.max.y);
}


}  // namespace math
