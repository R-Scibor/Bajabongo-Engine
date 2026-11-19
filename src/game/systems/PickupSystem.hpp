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
        ~PickupSystem();

        PickupSystem(const PickupSystem&) = delete;
        PickupSystem& operator=(const PickupSystem&) = delete;

    private:
        void onSensorBegin(const engine::PhysicsSensorBeginEvent& event);

        engine::EngineContext& m_context;
        std::shared_ptr<engine::ILogger> m_logger;
    };

} // namespace game
