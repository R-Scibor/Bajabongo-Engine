#pragma once
#include <string>

namespace engine {

    /**
     * @brief Represents the high-level logical state of an entity's animation.
     */
    enum class AnimationState {
        Idle,   ///< Entity is standing still.
        Walk,   ///< Entity is moving.
        Aim,    ///< Entity is aiming a weapon.
        Dead,   ///< Entity has been defeated/destroyed.
        Heal    ///< Entity is performing a healing action.
        // Future: Attack, etc.
    };

    /**
     * @brief Defines the horizontal facing direction for 2D sprites.
     */
    enum class FacingDirection {
        Right,  ///< Default facing direction (usually positive X).
        Left    ///< Flipped facing direction (usually negative X).
    };

    /**
     * @brief Component that drives the animation state machine.
     * * Handles the "Logic" side of animations (what represents the entity now),
     * as opposed to the AnimationComponent which handles the "Data" side (frames/timers).
     * It includes mechanisms to detect state changes between frames.
     */
    struct AnimationStateComponent {
        /// @brief Identifies which set of animations to use (e.g., "player", "zombie", "boss").
        std::string animationSetId; 
        
        /// @brief The current logical animation state.
        AnimationState state = AnimationState::Idle;
        
        /// @brief The current physical facing direction.
        FacingDirection facing = FacingDirection::Right;
        
        // --- State change detection members ---

        /// @brief The state recorded during the previous frame/update.
        AnimationState previousState = AnimationState::Idle;
        
        /// @brief The facing direction recorded during the previous frame/update.
        FacingDirection previousFacing = FacingDirection::Right;
        
        /// @brief The animation set ID recorded during the previous frame/update.
        std::string previousAnimationSetId;
        
        /**
         * @brief Checks if the animation state has changed since the last commit.
         * * @return true if state, facing, or animationSetId is different from the previous values.
         * @return false if no changes occurred.
         */
        bool stateChanged() const {
            return state != previousState || facing != previousFacing || animationSetId != previousAnimationSetId;
        }
        
        /**
         * @brief Synchronizes the previous state variables with the current ones.
         * * Should be called after the animation system has processed the changes
         * to prepare for the next frame's comparison.
         */
        void commitState() {
            previousState = state;
            previousFacing = facing;
            previousAnimationSetId = animationSetId;
        }
    };

} // namespace engine