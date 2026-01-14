#include "engine/pch.h"
#include "FogRenderSystem.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace game {

    FogRenderSystem::FogRenderSystem(engine::EngineContext& context)
        : m_context(context)
    {
        auto size = context.m_window->getSize();
        initializeTextures(size.x, size.y);
    }

    void FogRenderSystem::initializeTextures(unsigned int width, unsigned int height) {
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

        m_initialized = true;
    }

    void FogRenderSystem::resize(unsigned int width, unsigned int height) {
        initializeTextures(width, height);
    }

    void FogRenderSystem::update() {
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
        // We want to keep pixels that are already discovered (alpha > 0) OR currently visible.
        // A Max blend mode would work if we treat "Explored" as White and "Unexplored" as Transparent.
        // Let's assume Exploration Texture: Alpha 0 = Unexplored, Alpha 255 = Explored.
        
        sf::Sprite sightSprite(m_sightTexture.getTexture());
        // To accumulate, we can draw the sight sprite onto exploration texture.
        // If we want to keep what's already there, we need a blend mode that doesn't clear the destination.
        // sf::BlendNone would overwrite. sf::BlendAlpha would mix.
        // We want: Destination = Max(Destination, Source).
        // SFML standard blend modes don't have Max directly exposed easily without custom equation.
        // However, if we just draw the sight sprite (White) over the exploration texture...
        // If exploration texture has White (Explored) and we draw White (Visible), it stays White.
        // If exploration texture has Transparent (Unexplored) and we draw White, it becomes White.
        // So standard Alpha blending works if we draw opaque white on transparent!
        
        m_explorationTexture.draw(sightSprite);
        m_explorationTexture.display();
    }

    const sf::Texture& FogRenderSystem::getSightTexture() const {
        return m_sightTexture.getTexture();
    }

    const sf::Texture& FogRenderSystem::getExplorationTexture() const {
        return m_explorationTexture.getTexture();
    }

} // namespace game
