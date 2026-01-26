#include "engine/pch.h"
#include "LifetimeSystem.hpp"
#include "engine/core/EngineContext.hpp"

/**
 * @file LifetimeSystem.cpp
 * @brief Implementation of the LifetimeSystem class.
 */
#include "engine/components/LifetimeComponent.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <entt/entt.hpp>

namespace engine {

    LifetimeSystem::LifetimeSystem(EngineContext& context)
        : m_context(context)
    {
    }

    void LifetimeSystem::update(float fixedDeltaTime)
    {
        auto& registry = *m_context.m_registry;
        auto view = registry.view<LifetimeComponent>();
        auto logger = m_context.m_logManager->GetLogger("LifetimeSystem");

        view.each([&](entt::entity entity, LifetimeComponent& lifetime) {
            lifetime.lifetime -= fixedDeltaTime;
            if (lifetime.lifetime <= 0.0f) {
                if (logger) logger->debug("Entity {} expired due to lifetime.", entt::to_integral(entity));
                registry.destroy(entity);
            }
        });
    }

} // namespace engine