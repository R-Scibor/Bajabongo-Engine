#pragma once

namespace engine {

    /// @brief Supported animation states
    enum class AnimationState {
        Idle,
        Walk,
        // Future: Attack, Death, etc.
    };

    /// @brief Facing direction (2-directional)
    enum class FacingDirection {
        Right,  // Default facing
        Left    // Flipped
    };

    /// @brief Component that drives the animation state machine
    struct AnimationStateComponent {
        AnimationState state = AnimationState::Idle;
        FacingDirection facing = FacingDirection::Right;
        
        // Optional: State change detection
        AnimationState previousState = AnimationState::Idle;
        FacingDirection previousFacing = FacingDirection::Right;
        
        bool stateChanged() const {
            return state != previousState || facing != previousFacing;
        }
        
        void commitState() {
            previousState = state;
            previousFacing = facing;
        }
    };

} // namespace engine
