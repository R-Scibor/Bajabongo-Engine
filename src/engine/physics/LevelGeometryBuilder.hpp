#pragma once

#include <vector>
#include <memory>
#include <box2d/id.h>
#include <entt/entity/entity.hpp>
#include "engine/core/math/MathAliases.hpp"

namespace engine {
    struct EngineContext;
    class ILogger;

    /**
     * @brief Manages the creation of static level geometry in the physics world.
     *
     * Takes geometry chains (e.g., from GeometryBuilder) and creates a single static Box2D body
     * containing multiple chain shapes.
     */
    class LevelGeometryBuilder {
    public:
        explicit LevelGeometryBuilder(const EngineContext& context);
        ~LevelGeometryBuilder();

        /**
         * @brief Creates a static physics body from the provided chains.
         *
         * Destroys any previously created level body.
         *
         * @param chains A list of vertex chains defining the level geometry.
         */
        void createLevelBody(const std::vector<std::vector<Vector2f>>& chains);

        /**
         * @brief Destroys the level physics body.
         */
        void clear();

    private:
        b2WorldId m_worldId;
        b2BodyId m_levelBodyId;
        entt::registry& m_registry;
        entt::entity m_levelEntity{ entt::null };
        std::shared_ptr<ILogger> m_logger;
    };
}