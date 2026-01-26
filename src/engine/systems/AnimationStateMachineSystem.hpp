#pragma once
#include <memory>
#include <string>

namespace engine {
struct EngineContext;
class ILogger;
struct AnimationStateComponent;
enum class AnimationState;
enum class FacingDirection;

/**
 * @class AnimationStateMachineSystem
 * @brief Manages animation state transitions for entities.
 *
 * This system bridges the gap between high-level logical states (e.g., Walking, Idle, Attacking)
 * and low-level animation clips. It monitors the AnimationStateComponent and updates the
 * AnimationComponent with the correct clip ID based on the current state and facing direction.
 * It also handles visual flipping (scale inversion) for direction changes.
 */
class AnimationStateMachineSystem {
public:
    /**
     * @brief Constructs a new AnimationStateMachineSystem.
     *
     * @param context The engine context containing the registry and resource managers.
     */
    explicit AnimationStateMachineSystem(EngineContext& context);
    
    /**
     * @brief Updates the system.
     *
     * Checks for state changes in AnimationStateComponents and resolves the corresponding
     * animation clip to play. Also handles sprite flipping based on facing direction.
     */
    void update();

private:
    /** @brief Reference to the engine context. */
    EngineContext& m_context;
    
    /** @brief Logger instance. */
    std::shared_ptr<ILogger> m_logger;
    
    /**
     * @brief Resolves the animation clip ID for a given state and direction.
     *
     * Uses a fallback mechanism to find the most appropriate clip. For example,
     * if "player_ak47_idle" is missing, it might fall back to "player_idle".
     *
     * @param state The high-level animation state (Idle, Walk, etc.).
     * @param facing The direction the entity is facing.
     * @param entityType The base name of the animation set (e.g., "player").
     * @return std::string The resolved animation clip ID.
     */
    std::string getClipId(AnimationState state, FacingDirection facing, const std::string& entityType);
};

} // namespace engine
