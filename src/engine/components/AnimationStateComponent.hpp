#pragma once
#include <string>

namespace engine {

    /// @brief Supported animation states
    enum class AnimationState {
        Idle,
        Walk,
        Aim,
        Dead
        // Future: Attack, etc.
    };

    /// @brief Facing direction (2-directional)
    enum class FacingDirection {
        Right,  // Default facing
        Left    // Flipped
    };

    /// @brief Component that drives the animation state machine
    struct AnimationStateComponent {
        std::string animationSetId; // Identifies which set of animations to use (e.g., "player", "zombie")
        AnimationState state = AnimationState::Idle;
        FacingDirection facing = FacingDirection::Right;
        
        // Optional: State change detection
        AnimationState previousState = AnimationState::Idle;
        FacingDirection previousFacing = FacingDirection::Right;
        std::string previousAnimationSetId;
        
        bool stateChanged() const {
            return state != previousState || facing != previousFacing || animationSetId != previousAnimationSetId;
        }
        
        void commitState() {
            previousState = state;
            previousFacing = facing;
            previousAnimationSetId = animationSetId;
        }
    };

} // namespace engine
