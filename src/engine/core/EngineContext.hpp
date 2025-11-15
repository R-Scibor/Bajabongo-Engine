#pragma once

#include <box2d/id.h>

#include <entt/fwd.hpp>

// Forward declarations to avoid including heavy headers.
// The services that use the context will include the full headers they need.
namespace engine {

class ILoggerManager;
class IRenderer;
class IInputManager;
class IWindow;

/**
 * @brief A context object holding pointers to all major engine services.
 *
 * This struct is used to pass around all the core engine systems. It simplifies
 * constructors and allows services to pick and choose which dependencies they need
 * without requiring a long list of parameters.
 */
struct EngineContext {
    ILoggerManager* loggerManager = nullptr;
    entt::registry* registry = nullptr;
    b2WorldId physicsWorld{};
    IRenderer* renderer = nullptr;
    IInputManager* inputManager = nullptr;
    IWindow* window = nullptr;
};

} // namespace engine