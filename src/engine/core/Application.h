#pragma once

// Forward declarations of the interfaces to avoid including headers
class IWindow;
class IRenderer;

/**
 * @brief The main Application class.
 * This class owns all major engine services and orchestrates the main game loop.
 * It is decoupled from concrete implementations via abstract interfaces.
 */
class Application {
public:
    /**
     * @brief Constructs the Application, injecting its core service dependencies.
     * @param window A reference to an IWindow implementation.
     * @param renderer A reference to an IRenderer implementation.
     */
    Application(IWindow& window, IRenderer& renderer);
    ~Application();

    /**
     * @brief Starts and runs the main game loop.
     */
    void run();

private:
    /**
     * @brief Processes all pending inputs for the current frame.
     */
    void processInput();

    /**
     * @brief Updates the game state for the current frame.
     * @param deltaTime The time elapsed since the last frame, in seconds.
     */
    void update(float deltaTime);

    /**
     * @brief Renders the game state for the current frame.
     */
    void render();

    // References to the abstract interfaces for core services.
    IWindow& m_window;
    IRenderer& m_renderer;
};