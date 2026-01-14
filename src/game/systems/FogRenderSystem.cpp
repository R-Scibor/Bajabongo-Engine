#include "engine/pch.h"
#include "FogRenderSystem.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "engine/components/WorldBoundsComponent.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>

namespace game {

    FogRenderSystem::FogRenderSystem(engine::EngineContext& context)
        : m_context(context)
    {
        // Initial size (can be updated later when world bounds are found)
        auto size = context.m_window->getSize();
        initializeTextures(size.x, size.y);
    }

    void FogRenderSystem::initializeTextures(unsigned int width, unsigned int height) {
        // Prevent re-initialization if size hasn't changed significantly or is valid
        if (m_initialized && m_currentWidth == width && m_currentHeight == height) {
            return;
        }

        if (!m_sightTexture.resize({width, height})) {
            // Handle error
            if (m_context.m_logManager) {
               m_context.m_logManager->GetLogger("FogRenderSystem")->error("Failed to create sight render texture");
            }
        }
        
        if (!m_explorationTexture.resize({width, height})) {
            // Handle error
             if (m_context.m_logManager) {
               m_context.m_logManager->GetLogger("FogRenderSystem")->error("Failed to create exploration render texture");
            }
        } else {
             m_explorationTexture.clear(sf::Color(0, 0, 0, 0)); // Start fully transparent (unexplored)
             m_explorationTexture.display();
        }

        m_currentWidth = width;
        m_currentHeight = height;
        m_initialized = true;

        if (m_context.m_logManager) {
             m_context.m_logManager->GetLogger("FogRenderSystem")->info("Fog textures initialized to {}x{}", width, height);
        }
    }

    void FogRenderSystem::resize(unsigned int width, unsigned int height) {
        // Only resize if we are NOT using world bounds (e.g. initial fallback)
        // If we found world bounds, we ignore window resize events for texture size
        if (!m_usingWorldBounds) {
             initializeTextures(width, height);
        }
    }

    void FogRenderSystem::update() {
        // Check for WorldBoundsComponent to set correct texture size
        if (!m_usingWorldBounds) {
            auto boundsView = m_context.m_registry->view<engine::WorldBoundsComponent>();
            for (auto entity : boundsView) {
                const auto& bounds = boundsView.get<engine::WorldBoundsComponent>(entity);
                if (bounds.width > 0 && bounds.height > 0) {
                     initializeTextures(static_cast<unsigned int>(std::ceil(bounds.width)), static_cast<unsigned int>(std::ceil(bounds.height)));
                     m_usingWorldBounds = true;
                     break; // Use the first one found
                }
            }
        }

        if (!m_initialized) return;

        // 1. Clear sight texture to transparent
        m_sightTexture.clear(sf::Color::Transparent);

        // 2. Iterate entities with VisibilityComponent
        auto view = m_context.m_registry->view<VisibilityComponent>();
        
        // Use a VertexArray for drawing polygons to the render texture
        sf::VertexArray polygon(sf::PrimitiveType::TriangleFan);

        for (auto entity : view) {
            const auto& visibility = view.get<VisibilityComponent>(entity);
            const auto& points = visibility.visibilityPolygon;

            if (points.empty()) continue;

            polygon.resize(points.size());
            
            for (size_t i = 0; i < points.size(); ++i) {
                polygon[i].position = sf::Vector2f(points[i].x, points[i].y);
                polygon[i].color = sf::Color::White;
            }

            // Draw to sight texture with Additive blending (or just Overwrite if we want simple union)
            // Using Additive might make overlapping cones brighter, which might not be desired for a boolean mask.
            // Using None (copy) or Alpha (default) is safer for a flat mask. 
            // We want the union of all visible areas.
            // If we just draw white polygons on a transparent background, the result is the union.
            m_sightTexture.draw(polygon);
        }

        m_sightTexture.display();

        // 3. Accumulate into exploration texture
        // Draw the current sight texture onto the exploration texture
        // Use BlendAdd to accumulate opacity (White on Transparent)
        // Since sightTexture is White (Visible) on Transparent (Hidden), 
        // Adding it to explorationTexture (which starts Transparent) will accumulate the white pixels.
        // Once a pixel is white (1.0), adding more white keeps it white (clamped to 1.0).
        
        sf::Sprite sightSprite(m_sightTexture.getTexture());
        // We need to flip the sprite vertically because sf::RenderTexture is stored upside down relative to window
        // But here we are drawing from texture to texture, so coordinate systems might match?
        // Actually SFML RenderTextures are usually consistent with each other. 
        // Let's test without flipping first. If it's upside down, we'll fix it.
        // Wait, getTexture() returns the texture which is "right side up" for sprite drawing usually.
        
        // Use sf::BlendAdd to ensure we only "add" light.
        // If exploration has alpha 0, and sight has alpha 1 -> result alpha 1.
        // If exploration has alpha 1, and sight has alpha 0 -> result alpha 1.
        // This acts like a Boolean OR for visibility.
        m_explorationTexture.draw(sightSprite, sf::BlendAdd);
        m_explorationTexture.display();
    }

    const sf::Texture& FogRenderSystem::getSightTexture() const {
        return m_sightTexture.getTexture();
    }

    const sf::Texture& FogRenderSystem::getExplorationTexture() const {
        return m_explorationTexture.getTexture();
    }

} // namespace game
