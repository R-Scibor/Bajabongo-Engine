#pragma once

#include "engine/core/EngineContext.hpp"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <entt/entt.hpp>

namespace game {

    class FogRenderSystem {
    public:
        explicit FogRenderSystem(engine::EngineContext& context);

        void update();
        void resize(unsigned int width, unsigned int height);
        
        // Scene capture API
        void beginSceneCapture();
        void endSceneCapture();
        
        // Final composite render
        void renderFinal();
        
        const sf::Texture& getSightTexture() const;
        const sf::Texture& getExplorationTexture() const;

        // NEW: Provide access to scene texture for rendering
        sf::RenderTexture& getSceneRenderTexture();

    private:
        engine::EngineContext& m_context;
        
        sf::RenderTexture m_sightTexture;
        sf::RenderTexture m_explorationTexture;
        sf::RenderTexture m_sceneTexture;
        
        sf::Shader m_fogShader;
        
        bool m_initialized = false;
        bool m_shaderLoaded = false;
        bool m_usingWorldBounds = false;
        unsigned int m_currentWidth = 0;
        unsigned int m_currentHeight = 0;
        
        void initializeTextures(unsigned int width, unsigned int height);
    };

} // namespace game
