#pragma once
#include "engine/core/math/Vector2.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <vector>

namespace engine
{
    /**
     * @brief Collection of static geometric utility functions.
     */
    class GeometryUtils
    {
    public:
        /**
         * @brief Checks if a 2D point lies inside a polygon.
         * * Implements the Ray-casting algorithm (also known as the Even-odd rule).
         * Ideally, the polygon vertices should be ordered (clockwise or counter-clockwise),
         * but this algorithm works for simple non-intersecting polygons regardless of winding.
         *
         * @param point The 2D point to test.
         * @param polygon A list of vertices defining the polygon edges.
         * @return true if the point is strictly inside the polygon, false otherwise.
         */
        static bool isPointInPolygon(const Vector2f& point, const std::vector<Vector2f>& polygon)
        {
            if (polygon.size() < 3) return false;
            
            bool inside = false;
            size_t n = polygon.size();
            
            for (size_t i = 0, j = n - 1; i < n; j = i++)
            {
                const auto& vi = polygon[i];
                const auto& vj = polygon[j];
                
                // Ray-casting test: Check if the ray from 'point' along the X-axis intersects the edge (vi, vj)
                if (((vi.y > point.y) != (vj.y > point.y)) &&
                    (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x))
                {
                    inside = !inside;
                }
            }
            
            return inside;
        }
    };
}