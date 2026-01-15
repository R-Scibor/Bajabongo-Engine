#pragma once
#include "engine/core/math/Vector2.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <vector>

namespace engine
{
    class GeometryUtils
    {
    public:
        // Ray-casting algorithm for point-in-polygon test
        static bool isPointInPolygon(const Vector2f& point, const std::vector<Vector2f>& polygon)
        {
            if (polygon.size() < 3) return false;
            
            bool inside = false;
            size_t n = polygon.size();
            
            for (size_t i = 0, j = n - 1; i < n; j = i++)
            {
                const auto& vi = polygon[i];
                const auto& vj = polygon[j];
                
                // Ray-casting test
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
