#pragma once
#include <memory>
#include <string>

namespace engine {
struct EngineContext;
class ILogger;
struct AnimationStateComponent;
enum class AnimationState;
enum class FacingDirection;

/// @brief Maps (State, Direction) -> AnimationClipId
class AnimationStateMachineSystem {
public:
    explicit AnimationStateMachineSystem(EngineContext& context);
    
    void update();

private:
    EngineContext& m_context;
    std::shared_ptr<ILogger> m_logger;
    
    // Helper to build clip ID from state + direction
    std::string getClipId(AnimationState state, FacingDirection facing, const std::string& entityType);
};

} // namespace engine
