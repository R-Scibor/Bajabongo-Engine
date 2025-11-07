#include "Application.h"
#include "engine/core/IWindow.h"
#include "engine/rendering/IRenderer.h"
#include <chrono> // For high-resolution clock and delta time

Application::Application(IWindow& window, IRenderer& renderer)
    : m_window(window), m_renderer(renderer) {
    // Dependencies are injected and stored as references
}

Application::~Application() {
    // No explicit cleanup needed here, services are owned by the composition root
}

void Application::run() {
    // Note: m_window.create() is now called in main.cpp *before* this.

    auto lastTime = std::chrono::high_resolution_clock::now();

    // The main game loop
    while (m_window.isOpen()) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Execute frame logic
        processInput();
        update(deltaTime);
        render();
    }
}

void Application::processInput() {
    // Delegate event processing to the window
    m_window.pollEvents();
}

void Application::update(float deltaTime) {
    // (void)deltaTime; // Suppress unused parameter warning
    // In Phase 1, there is no game state to update.
    // This will be filled in later (e.g., physics, AI).
}

void Application::render() {
    // Orchestrate the rendering process via the abstract interface
    m_renderer.beginFrame();
    m_renderer.clear({ 0, 0, 25, 255 }); // Dark blue background
    m_renderer.drawShape(); // The Phase 1 milestone
    m_renderer.endFrame();
}