# Logging System Architecture

The Bajabongo Engine features a robust, dependency-injection-based logging system. It is designed to be highly configurable, decoupled from the engine's core, and easy to integrate into any module.

## Key Features

1.  **Decoupled via Interfaces**: The system is built around two core interfaces, `ILogger` and `ILoggerManager`. This decouples the engine from any specific logging library, allowing the implementation to be swapped out without affecting the engine's source code.

2.  **Dependency Injection**: Loggers are not accessed globally. Instead, the `ILoggerManager` is injected into systems during their construction. Those systems then request their own named `ILogger` instance, promoting clear and manageable dependencies.

3.  **Runtime Configuration**: Log verbosity is controlled externally via the `config/logging.ini` file. This allows developers to adjust the log output for different modules at runtime without needing to recompile the engine.

4.  **High-Performance Formatted Logging**: The logger uses the `{fmt}` library for fast, type-safe, and compile-time-checked string formatting, avoiding the runtime overhead of `std::stringstream`.

## How It Works

### System Integration

The logging system is initialized in the application's entry point (`game.cpp`) and passed down to other systems.

1.  **Initialization**: An instance of `SpdlogManager` (the concrete implementation of `ILoggerManager`) is created in `main()`.
2.  **Injection**: A reference to the manager (`ILoggerManager&`) is passed to the `Application` and subsequently to any other systems that require logging.
3.  **Logger Retrieval**: Each system requests its own dedicated, named logger from the manager, for example: `m_logger = logManager.GetLogger("Physics");`.

### Log Level Filtering

The system uses a threshold-based filter to control log output. The level set in `config/logging.ini` determines the *minimum* severity required for a message to be displayed.

**Hierarchy (from least to most severe):**
`trace` → `debug` → `info` → `warn` → `error`

**Example:**
If a logger's level is set to `info` (`Core = info`), it will display messages logged with `info()`, `warn()`, and `error()`, but it will ignore `trace()` and `debug()` messages.

If a logger is requested that is not defined in the `.ini` file, it automatically defaults to the `info` level.

## How to Use

The logger provides a modern, type-safe formatting API. This is the preferred way to log messages with variables.

To add logging to a new system:

1.  Modify the system's constructor to accept a reference to `engine::ILoggerManager&`.
2.  In the constructor, request a logger with a unique name for that system.
3.  Use the logger instance to write messages with `{}` placeholders for variables.

```cpp
// Example in a hypothetical PhysicsSystem
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

class PhysicsSystem
{
public:
    PhysicsSystem(engine::ILoggerManager& logManager)
    {
        // Request a logger named "Physics"
        m_logger = logManager.GetLogger("Physics");
        m_logger->info("PhysicsSystem initialized.");
    }

    void Update(float deltaTime)
    {
        int rigidBodyCount = 120;
        // Use formatted logging for variables. It's clean and type-safe.
        m_logger->debug("Running physics simulation step for {} rigid bodies with dt = {:.4f}s.", rigidBodyCount, deltaTime);
        // ...
    }

private:
    std::shared_ptr<engine::ILogger> m_logger;
};
```

### Backwards Compatibility

For simplicity or compatibility with older code, you can still pass a single string or string view. This is useful for static messages without variables.

```cpp
// This is still valid for messages without variables.
m_logger->trace("Running physics simulation step.");