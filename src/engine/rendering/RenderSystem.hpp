#pragma once

#include <entt/fwd.hpp>
#include <memory>
#include <SFML/Graphics/RenderTarget.hpp>

namespace engine
{
    struct EngineContext;
    class IRenderer;
    class SpriteManager;
    class IResourceManager;

    /**
     * @brief System responsible for drawing all visible entities to the screen.
     * * Handles:
     * 1. Culling (filtering invisible objects).
     * 2. Sorting (Painter's Algorithm based on Layer and Y-position).
     * 3. Dispatching draw calls to the IRenderer or RenderTarget.
     * 4. Debug visualization (colliders, wires).
     */
    class RenderSystem
    {
    public:
        explicit RenderSystem(engine::EngineContext& context);

        /**
         * @brief Main rendering loop for the ECS.
         * @param target The SFML render target (Window or RenderTexture).
         */
        void update(sf::RenderTarget& target);

        /**
         * @brief Toggles drawing of physics debug shapes (colliders, sensors).
         */
        void setDebugDraw(bool enable);

    private:
        engine::EngineContext& m_context;
        entt::registry& m_registry;
        std::shared_ptr<engine::IRenderer> m_renderer;
        std::shared_ptr<engine::SpriteManager> m_spriteManager;
        std::shared_ptr<engine::IResourceManager> m_resourceManager;
        
        /// @brief Flag controlling physics debug rendering.
        bool m_debugDraw = false;
    };
}