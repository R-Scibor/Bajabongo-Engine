#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/events/PhysicsEvents.hpp"

namespace game {

    class DamageSystem {
    public:
        DamageSystem(engine::EngineContext& context);
        ~DamageSystem();

        // No update needed, strictly event-based
        
        void onSensorBegin(const engine::PhysicsSensorBeginEvent& event);
        void onContactBegin(const engine::PhysicsContactBeginEvent& event);

    private:
        void handleCollision(entt::entity projectile, entt::entity target, bool isTargetSensor, entt::registry& registry);
        void spawnDeadBody(entt::entity originalEntity, entt::registry& registry);

        engine::EngineContext& m_context;
        std::shared_ptr<engine::ILogger> m_logger;
    };

}