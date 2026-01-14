#pragma once

#include "engine/core/EngineContext.hpp"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <entt/entt.hpp>

namespace game {

    class FogRenderSystem {
    public:
        explicit FogRenderSystem(engine::EngineContext& context);

        void update();
        void resize(unsigned int width, unsigned int height);
        
        const sf::Texture& getSightTexture() const;
        const sf::Texture& getExplorationTexture() const;

    private:
        engine::EngineContext& m_context;
        
        sf::RenderTexture m_sightTexture;
        sf::RenderTexture m_explorationTexture;
        
        bool m_initialized = false;
        
        void initializeTextures(unsigned int width, unsigned int height);
    };

} // namespace game
