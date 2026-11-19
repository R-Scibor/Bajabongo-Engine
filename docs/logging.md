# Logging System Architecture

The Bajabongo Engine features a robust, dependency-injection-based logging system. It is designed to be highly configurable, decoupled from the engine's core, and easy to integrate into any module.

## Key Features

1.  **Decoupled via Interfaces**: The system is built around two core interfaces, `ILogger` and `ILoggerManager`. This decouples the engine from any specific logging library, allowing the implementation to be swapped out without affecting the engine's source code.

2.  **Dependency Injection**: Loggers are not accessed globally. Instead, the `ILoggerManager` is injected into systems during their construction (directly, or through `EngineContext::m_logManager`). Those systems then request their own named `ILogger` instance, promoting clear and manageable dependencies.

3.  **Runtime Configuration**: Log verbosity is controlled externally via the `config/logging.ini` file. This allows developers to adjust the log output for different modules at runtime without needing to recompile the engine.

4.  **High-Performance Formatted Logging**: The logger uses the `{fmt}` library for fast, type-safe, and compile-time-checked string formatting, avoiding the runtime overhead of `std::stringstream`.

5.  **Log File**: Every logger writes to the console and to `logs/game.log`, relative to the working directory (`out/game/<Config>` when run from the build tree). The directory is created if missing and the file is truncated on each start. Warnings and errors are flushed at once, and everything else at least once per second, so a crash still leaves the log on disk. If the file cannot be opened, the game logs to the console only.

## How It Works

### System Integration

The logging system is initialized in the application's entry point (`game.cpp`) and passed down to other systems.

1.  **Initialization**: An instance of `SpdlogManager` (the concrete implementation of `ILoggerManager`) is created in `main()`.
2.  **Injection**: The manager is stored as a `std::shared_ptr<ILoggerManager>` in `EngineContext::m_logManager`. `Application`, states, and systems read it from the context; a few services (such as `SFMLInputManager`) take an `ILoggerManager&` in their constructor.
3.  **Logger Retrieval**: Each system requests its own dedicated, named logger from the manager, for example: `m_logger = logManager.getLogger("Physics");`.

### Log Level Filtering

The system uses a threshold-based filter to control log output. The level set in `config/logging.ini` determines the *minimum* severity required for a message to be displayed.

**Hierarchy (from least to most severe):**
`trace` → `debug` → `info` → `warn` → `error` → `critical`

**Example:**
If a logger's level is set to `info` (`Core = info`), it will display messages logged with `info()`, `warn()`, and `error()`, but it will ignore `trace()` and `debug()` messages.

If a logger is requested that is not defined in the `.ini` file, it automatically defaults to the `info` level. Unknown level names in the file also fall back to `info`.

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
        m_logger = logManager.getLogger("Physics");
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