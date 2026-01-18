#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/events/PhysicsEvents.hpp"
#include <memory>

namespace engine {
    class ILogger;
}

namespace game {

    class PickupSystem {
    public:
        explicit PickupSystem(engine::EngineContext& context);
        ~PickupSystem() = default;

    private:
        void onSensorBegin(const engine::PhysicsSensorBeginEvent& event);

        engine::EngineContext& m_context;
        std::shared_ptr<engine::ILogger> m_logger;
    };

}
