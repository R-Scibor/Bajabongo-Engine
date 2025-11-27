#include "engine/pch.h"
#include "GeometryBuilder.hpp"
#include "engine/core/ILogger.hpp"
#include <algorithm>
#include <cmath>

namespace engine {

    GeometryBuilder::GeometryBuilder(std::shared_ptr<ILogger> logger)
        : m_logger(std::move(logger))
    {
    }

    std::vector<std::vector<Vector2f>> GeometryBuilder::analyzeGrid(const std::vector<std::vector<int>>& grid, float tileSize) {
        std::vector<std::vector<Vector2f>> result;
        if (grid.empty()) {
            if (m_logger) m_logger->warn("GeometryBuilder: Grid is empty!");
            return result;
        }

        size_t height = grid.size();
        size_t width = grid[0].size();

        if (m_logger) {
            m_logger->info("GeometryBuilder: Analyzing grid of size {}x{} with tileSize {}", width, height, tileSize);
        }

        auto islands = findIslands(grid);
        if (m_logger) {
            m_logger->info("GeometryBuilder: Found {} islands.", islands.size());
        }

        for (const auto& island : islands) {
            auto contour = traceContour(island, grid, tileSize);
            auto simplified = simplifyContour(contour);
            if (m_logger) {
                m_logger->debug("  Island contour: {} vertices -> Simplified: {} vertices", contour.size(), simplified.size());
            }
            result.push_back(simplified);
        }
        return result;
    }

    std::vector<GeometryBuilder::TileIsland> GeometryBuilder::findIslands(const std::vector<std::vector<int>>& grid) {
        std::vector<TileIsland> islands;
        if (grid.empty()) return islands;

        int height = static_cast<int>(grid.size());
        int width = static_cast<int>(grid[0].size());

        std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

        // Helper directions for 4-connectivity
        const int dx[] = { 1, -1, 0, 0 };
        const int dy[] = { 0, 0, 1, -1 };

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // If it's a wall (1) and not visited yet
                if (grid[y][x] == 1 && !visited[y][x]) {
                    TileIsland island;
                    std::vector<Vector2i> stack;
                    stack.push_back({ x, y });
                    visited[y][x] = true;

                    while (!stack.empty()) {
                        Vector2i current = stack.back();
                        stack.pop_back();
                        island.tiles.push_back(current);

                        for (int i = 0; i < 4; ++i) {
                            int nx = current.x + dx[i];
                            int ny = current.y + dy[i];

                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                if (grid[ny][nx] == 1 && !visited[ny][nx]) {
                                    visited[ny][nx] = true;
                                    stack.push_back({ nx, ny });
                                }
                            }
                        }
                    }
                    islands.push_back(island);
                }
            }
        }

        return islands;
    }

    std::vector<Vector2f> GeometryBuilder::traceContour(const TileIsland& island, const std::vector<std::vector<int>>& grid, float tileSize) {
        std::vector<Vector2f> vertices;
        if (island.tiles.empty()) return vertices;

        // 1. Find top-leftmost tile in the island
        Vector2i startTile = island.tiles[0];
        for (const auto& tile : island.tiles) {
            if (tile.y < startTile.y || (tile.y == startTile.y && tile.x < startTile.x)) {
                startTile = tile;
            }
        }

        // Start vertex is the top-left corner of the startTile
        Vector2i currentVertex = { startTile.x, startTile.y };
        // Initial direction is South (Down) - keeping wall (startTile) to our Left
        Vector2i dir = { 0, 1 };
        
        Vector2i startVertex = currentVertex;
        Vector2i startDir = dir;

        bool firstMove = true;

        // Helper to check if a coordinate is a solid wall
        auto isSolid = [&](int x, int y) -> bool {
            if (y < 0 || y >= grid.size()) return false;
            if (x < 0 || x >= grid[0].size()) return false;
            return grid[y][x] == 1;
        };

        // Directions: N, E, S, W
        // We use (dx, dy)
        // "Left" relative to direction (dx, dy):
        // if (0, 1) [S] -> Left is (1, 0) [E]? No.
        // Cross product logic or manual mapping.
        // Rotations (CCW on screen Y-down):
        // S(0,1) -> E(1,0) -> N(0,-1) -> W(-1,0) -> S... This is CCW rotation of direction vector.
        
        // Correct "Left Turn" for checking tiles (keeping wall on left):
        // We are on an edge.
        // Dir S (0,1). Left Tile is relative (0, 0) from vertex if moving from (0,-1).
        // Let's rely on the vertex probe logic.

        int height = static_cast<int>(grid.size());
        int width = static_cast<int>(grid[0].size());

        int maxIterations = width * height * 4; // Safety break
        int iterations = 0;

        do {
            vertices.push_back({ static_cast<float>(currentVertex.x) * tileSize, static_cast<float>(currentVertex.y) * tileSize });

            // Move to next vertex
            currentVertex.x += dir.x;
            currentVertex.y += dir.y;

            // Determine next direction at this new vertex
            // We want to keep "Solid" on our Left.
            // Current edge was `dir`.
            // Check Left Turn (CCW), Straight, Right Turn (CW), Back.
            
            // Defined directions for rotation
            Vector2i leftTurn = { dir.y, -dir.x };   // (0,1)->(1,0) E
            Vector2i straight = dir;
            Vector2i rightTurn = { -dir.y, dir.x };  // (0,1)->(-1,0) W
            Vector2i back = { -dir.x, -dir.y };

            // Which tile is to the "Left" of an edge starting at `currentVertex` with direction `D`?
            // Edge from (vx, vy) to (vx+dx, vy+dy).
            // The tile "Left" of this edge is:
            // If D=(0,1) [S], Left is (vx, vy).
            // If D=(1,0) [E], Left is (vx, vy-1).
            // If D=(0,-1) [N], Left is (vx-1, vy-1).
            // If D=(-1,0) [W], Left is (vx-1, vy).
            
            // Get tile coordinates on the Left and Right of an edge starting at p with direction d
            auto getLeftTile = [&](Vector2i p, Vector2i d) -> Vector2i {
                int tx = p.x;
                int ty = p.y;
                if (d.x == 1) ty -= 1;
                else if (d.x == -1) tx -= 1;
                else if (d.y == -1) { tx -= 1; ty -= 1; }
                // else if (d.y == 1) no offset needed
                return { tx, ty };
            };

            auto getRightTile = [&](Vector2i p, Vector2i d) -> Vector2i {
                int tx = p.x;
                int ty = p.y;
                if (d.x == 1) { /* no offset */ }
                else if (d.x == -1) { tx -= 1; ty -= 1; }
                else if (d.y == -1) tx -= 1;
                else if (d.y == 1) ty -= 1;
                return { tx, ty };
            };
            
            auto isValidEdge = [&](Vector2i p, Vector2i d) -> bool {
                Vector2i l = getLeftTile(p, d);
                Vector2i r = getRightTile(p, d);
                return isSolid(l.x, l.y) && !isSolid(r.x, r.y);
            };

            // Priority: Left Turn (Convex corner), Straight, Right Turn (Concave corner), Back.
            if (isValidEdge(currentVertex, leftTurn)) {
                 dir = leftTurn;
            } else if (isValidEdge(currentVertex, straight)) {
                 dir = straight;
            } else if (isValidEdge(currentVertex, rightTurn)) {
                 dir = rightTurn;
            } else {
                 dir = back; // U-turn
            }

            firstMove = false;
            iterations++;

        } while ((currentVertex.x != startVertex.x || currentVertex.y != startVertex.y) && iterations < maxIterations);

        if (iterations >= maxIterations && m_logger) {
            m_logger->error("GeometryBuilder: Contour tracing exceeded max iterations!");
        }

        return vertices;
    }

    std::vector<Vector2f> GeometryBuilder::simplifyContour(const std::vector<Vector2f>& points) {
        if (points.size() < 3) return points;

        std::vector<Vector2f> simplified;
        simplified.push_back(points[0]);

        for (size_t i = 1; i < points.size(); ++i) {
            // Previous point (last added to simplified)
            Vector2f p1 = simplified.back();
            // Current candidate
            Vector2f p2 = points[i];
            
            // If we are at the last point, always add it
            if (i == points.size() - 1) {
                simplified.push_back(p2);
                break;
            }

            // Next point
            Vector2f p3 = points[i + 1];

            // Check if p2 is collinear with p1 and p3
            // Vector p1->p2
            float dx1 = p2.x - p1.x;
            float dy1 = p2.y - p1.y;
            // Vector p2->p3
            float dx2 = p3.x - p2.x;
            float dy2 = p3.y - p2.y;

            // Cross product z-component: dx1*dy2 - dx2*dy1
            float cross = dx1 * dy2 - dx2 * dy1;

            // If cross product is near zero, points are collinear
            if (std::abs(cross) > 0.001f) {
                // Not collinear, keep p2
                simplified.push_back(p2);
            }
            // Else: collinear, skip p2 (don't add it, next iteration will check p1->p3)
        }

        // Handle loop closure if start == end
        // If original was closed, simplified should be closed
        // But my trace logic might return duplicate start/end or not.
        // Current logic: traceContour stops when currentVertex == startVertex
        // But traceContour ADDS the currentVertex at the start of loop.
        // It does NOT add the final vertex because it breaks the loop.
        // So start == end is implicit if we loop.
        
        // Let's check if first and last points of simplified are collinear with second point?
        // For now, basic simplification is enough.

        return simplified;
    }

}