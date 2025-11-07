#include "engine/core/Application.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/logging/SpdlogManager.hpp"
#include "engine/core/ILogger.hpp"
#include <iostream>

int main() {
    try {
        Bajabongo::SpdlogManager logManager("config/logging.ini");
        auto gameLogger = logManager.GetLogger("Game");

        gameLogger->info("Game is initializing...");
        gameLogger->trace("This is a detailed trace message from the game.");
        gameLogger->debug("This is a debug message, useful for development.");

        SFMLRenderer renderer;
        auto rendererLogger = logManager.GetLogger("Renderer");
        rendererLogger->warn("SFML Renderer is being created. This is a warning.");

        renderer.create("Game Window", 1280, 720);
        gameLogger->info("Window created successfully.");

        Application app(renderer, renderer, logManager);

        app.run();

        gameLogger->error("This is a fake error message after the game loop finishes.");
    }
    catch (const std::exception& e) {
        // In a real scenario, we would get a logger and log the exception.
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}