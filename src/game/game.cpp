#include "engine/core/Application.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/input/SFMLInputManager.hpp"
#include "engine/logging/SpdlogManager.hpp"
#include "engine/core/ILogger.hpp"
#include <iostream>

int main() {
    try {
        Bajabongo::SpdlogManager logManager("config/logging.ini");
        auto gameLogger = logManager.GetLogger("Game");

        gameLogger->info("Game is initializing...");
        SFMLRenderer renderer;

        renderer.create("Game Window", 1280, 720);
        gameLogger->info("Window created successfully.");

        engine::SFMLInputManager inputManager(logManager);

        Application app(renderer, renderer, logManager, inputManager);

        app.run();

        gameLogger->info("Game shutting down.");
    }
    catch (const std::exception& e) {
        // In a real scenario, we would get a logger and log the exception.
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}