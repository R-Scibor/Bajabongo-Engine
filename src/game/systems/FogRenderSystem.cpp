#include "engine/pch.h"
#include "FogRenderSystem.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "engine/components/WorldBoundsComponent.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cmath>

namespace game {

    FogRenderSystem::FogRenderSystem(engine::EngineContext& context)
        : m_context(context)
    {
        // TEMPORARY INITIALIZATION: Use window size as fallback
        // This will be replaced by world bounds on first update() call
        // when MapLoader adds WorldBoundsComponent to the map entity
        auto size = context.m_window->getSize();
        initializeTextures(size.x, size.y);
        
        if (m_context.m_logManager) {
            auto logger = m_context.m_logManager->GetLogger("FogRenderSystem");
            logger->warn("FogRenderSystem: Initialized with window size {}x{} (temporary). "
                         "Will resize to world bounds automatically.",
                         size.x, size.y);
        }
        
        // Load fog shader
        // Try sf::Shader::Type::Fragment if available, otherwise just rely on context
        if (!m_fogShader.loadFromFile("../../assets/shaders/fog.frag", sf::Shader::Type::Fragment)) {
            if (m_context.m_logManager) {
                auto logger = m_context.m_logManager->GetLogger("FogRenderSystem");
                logger->error("Failed to load fog.frag shader! Fog will not render correctly.");
            }
            m_shaderLoaded = false;
        } else {
            m_shaderLoaded = true;
            if (m_context.m_logManager) {
                auto logger = m_context.m_logManager->GetLogger("FogRenderSystem");
                logger->info("Fog shader loaded successfully.");
            }
        }
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

        if (!m_sceneTexture.resize({width, height})) {
             if (m_context.m_logManager) {
               m_context.m_logManager->GetLogger("FogRenderSystem")->error("Failed to create scene render texture");
            }
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

    sf::RenderTexture& FogRenderSystem::getSceneRenderTexture()
    {
        return m_sceneTexture;
    }

    void FogRenderSystem::beginSceneCapture()
    {
        if (!m_initialized) return;
        
        m_sceneTexture.clear(sf::Color::Black);
        
        // CRITICAL: Set view to world space (entire map), not viewport
        if (m_usingWorldBounds) {
            // Create a view that covers the entire world/map
            sf::View worldView(sf::FloatRect(
                {0.f, 0.f},
                {static_cast<float>(m_currentWidth), static_cast<float>(m_currentHeight)}
            ));
            m_sceneTexture.setView(worldView);
        } else {
            // Fallback: If world bounds not loaded yet, use window view
            auto sfmlRenderer = std::dynamic_pointer_cast<engine::SFMLRenderer>(m_context.m_renderer);
            if (sfmlRenderer) {
                m_sceneTexture.setView(sfmlRenderer->getNativeRenderWindow().getView());
            }
        }
    }

    void FogRenderSystem::endSceneCapture()
    {
        if (!m_initialized) return;
        
        // Finalize the render texture
        m_sceneTexture.display();
    }

    void FogRenderSystem::renderFinal()
    {
        if (!m_initialized) {
            return;
        }
        
        // Fallback: If shader failed to load, render scene without fog
        if (!m_shaderLoaded) {
            auto sfmlRenderer = std::dynamic_pointer_cast<engine::SFMLRenderer>(m_context.m_renderer);
            if (sfmlRenderer) {
                sf::Sprite sceneSprite(m_sceneTexture.getTexture());
                sceneSprite.setPosition(sf::Vector2f(0.f, 0.f));
                sfmlRenderer->getNativeRenderWindow().draw(sceneSprite);
            }
            return;
        }
        
        // Set shader uniforms
        m_fogShader.setUniform("sceneTexture", m_sceneTexture.getTexture());
        m_fogShader.setUniform("sightMap", m_sightTexture.getTexture());
        m_fogShader.setUniform("explorationMap", m_explorationTexture.getTexture());
        
        // Create a sprite that represents the entire world
        sf::Sprite worldSprite(m_sceneTexture.getTexture());
        worldSprite.setPosition(sf::Vector2f(0.f, 0.f));  // World origin
        
        // Prepare render states with shader
        sf::RenderStates states;
        states.shader = &m_fogShader;
        
        // Draw to window - the camera view will automatically transform world to screen
        auto sfmlRenderer = std::dynamic_pointer_cast<engine::SFMLRenderer>(m_context.m_renderer);
        if (sfmlRenderer) {
            // The window's current view (set by CameraSystem) handles world-to-screen transform
            sfmlRenderer->getNativeRenderWindow().draw(worldSprite, states);
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
                     break; 
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

        
        // Use sf::BlendAdd to ensure we only "add" light.
        // If exploration has alpha 0, and sight has alpha 1 -> result alpha 1.
        // If exploration has alpha 1, and sight has alpha 0 -> result alpha 1.
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
