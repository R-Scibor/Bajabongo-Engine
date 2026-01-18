#include "engine/pch.h"
#include "PickupSystem.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/MedkitPickupComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include <entt/entt.hpp>
#include <algorithm>

namespace game {

    PickupSystem::PickupSystem(engine::EngineContext& context)
        : m_context(context) {
        m_logger = context.m_logManager->GetLogger("PickupSystem");
        
        // Subscribe to sensor events
        context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>()
            .connect<&PickupSystem::onSensorBegin>(this);
    }

    void PickupSystem::onSensorBegin(const engine::PhysicsSensorBeginEvent& event) {
        auto& reg = *m_context.m_registry;
        
        // Check: Is visitor == Player AND sensor == MedkitPickup?
        // Note: sensorEntity is the entity with the sensor fixture
        if (!reg.valid(event.visitorEntity) || !reg.valid(event.sensorEntity)) return;

        bool isPlayer = reg.all_of<PlayerComponent>(event.visitorEntity);
        bool isPickup = reg.all_of<MedkitPickupComponent>(event.sensorEntity);

        if (isPlayer && isPickup) {
            auto& player = reg.get<PlayerComponent>(event.visitorEntity);
            auto& pickup = reg.get<MedkitPickupComponent>(event.sensorEntity);
            
            // Apply pickup
            if (player.medkits < player.maxMedkits) {
                int given = std::min(pickup.medkitsGiven, player.maxMedkits - player.medkits);
                player.medkits += given;
                
                m_logger->info("Player picked up {} medkit(s). Total: {}/{}", given, player.medkits, player.maxMedkits);
                
                // Destroy pickup entity
                reg.destroy(event.sensorEntity);
            } else {
                // m_logger->debug("Player inventory full. Medkit not picked up.");
            }
        }
    }

}
