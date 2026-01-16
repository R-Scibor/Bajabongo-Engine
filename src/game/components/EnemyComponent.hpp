// File: src/game/components/EnemyComponent.hpp
#pragma once
#include "engine/core/math/MathAliases.hpp"
#include <entt/entity/entity.hpp>

namespace game {

enum class EnemyState {
    Idle,
    Patrol,
    Alert,
    Approach,
    Chase,
    Rush,
    Shoot,
    Camp,
    Retreat,
    Hide,
    Turret,
    Dead
};

struct EnemyComponent {
    // State Machine
    EnemyState currentState = EnemyState::Idle;
    float stateTimer = 0.0f;
    
    // Detection & Targeting
    float detectionRadius = 400.0f;
    float alertRadius = 500.0f;
    float attackRange = 300.0f;
    entt::entity targetEntity = entt::null;
    engine::Vector2f lastKnownPosition{0.0f, 0.0f};
    float lastSeenTimer = 0.0f;
    float memoryDuration = 5.0f;
    
    // Movement
    float moveSpeed = 80.0f;
    
    // Combat
    float retreatHpPercent = 0.3f;
    
    // Stuck Detection
    float stuckCheckTimer = 0.0f;
    float stuckCheckInterval = 2.0f;
    engine::Vector2f lastCheckedPosition{0.0f, 0.0f};
    int stuckCounter = 0;
    engine::Vector2f lastValidPosition{0.0f, 0.0f};
    
    // Recovery
    float stunTimer = 0.0f; // Disables movement updates when > 0
    
    // Flags
    bool hasLineOfSight = false;
    bool isAlerted = false;
};

} // namespace game
