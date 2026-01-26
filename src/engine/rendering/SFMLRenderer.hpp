#pragma once
#include "engine/core/math/MathAliases.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/IWindow.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <memory>
#include <optional>

namespace engine
{
    class ILogger;
}

namespace engine
{
    /**
     * @brief Concrete implementation of the Rendering and Windowing subsystems using SFML.
     * * This class acts as a bridge (Adapter pattern) between the engine's abstract interfaces
     * (`IRenderer`, `IWindow`) and the concrete `sf::RenderWindow` class.
     * * @note Since SFML's RenderWindow handles both window management (OS events) and 
     * drawing, this class implements both interfaces simultaneously.
     */
    class SFMLRenderer : public IRenderer, public IWindow {
    public:
        SFMLRenderer();
        ~SFMLRenderer() override;

        // --- IWindow Implementation ---

        /**
         * @brief Creates the OS window using SFML.
         * @param title Title bar text.
         * @param width Width in pixels.
         * @param height Height in pixels.
         * @param fullscreen If true, attempts to switch to fullscreen mode.
         */
        void create(const std::string& title, unsigned int width, unsigned int height, bool fullscreen = false) override;
        
        /// @brief Closes the window and destroys the context.
        void close() override;
        
        /// @brief Checks if the window is open/valid.
        bool isOpen() const override;
        
        /**
         * @brief Gets the OS-specific handle (HWND, etc.).
         * * Crucial for integrating third-party tools like ImGui that need native access.
         */
        void* getNativeHandle() const override;
        
        /// @brief Polls the SFML event queue.
        std::optional<sf::Event> pollEvent() override;
        
        /// @brief Returns window dimensions.
        Vector2u getSize() const override;
        
        /// @brief Caps the frame rate to reduce CPU/GPU usage.
        void setFramerateLimit(unsigned int limit) override;

        // --- IRenderer Implementation ---

        /// @brief Prepares the window for a new frame (activates context).
        void beginFrame() override;
        
        /// @brief Clears the entire buffer with a solid color.
        void clear(Color color) override;

        // --- Primitives (Used mostly for Debugging) ---
        
        void drawCircle(engine::Vector2f position, float radius) override;
        void drawRect(engine::Vector2f position, engine::Vector2f size) override;
        void drawRect(engine::Vector2f position, engine::Vector2f size, Color color) override;
        void drawText(const std::string& text, engine::Vector2f position, uint32_t fontSize, Color color) override;
        void drawLine(engine::Vector2f start, engine::Vector2f end, Color color) override;
        void drawPolygon(const std::vector<engine::Vector2f>& vertices, const Color& color) override;

        // --- Texture & Sprite Handling ---

        /**
         * @brief Draws a raw texture pointer.
         * * Casts the void* handle back to `sf::Texture`.
         * @warning Unsafe: Ensure the handle passed is a valid `sf::Texture*`.
         */
        void drawTexture(const void* textureHandle, engine::Vector2f position) override;
        void drawTexture(const void* textureHandle, engine::Vector2f position, engine::Vector2f size) override;
        
        /**
         * @brief Draws a raw sprite pointer.
         * * Used for bypassing the ECS component system (e.g., debug overlays).
         */
        void drawSpriteDirect(const void* spriteHandle, engine::Vector2f position) override;
        
        /**
         * @brief The main drawing method for ECS entities.
         * * Uses `IResourceManager` to resolve the `spriteId` into a concrete texture.
         */
        void drawSprite(const SpriteDesc& sprite, const TransformComponent& transform, const Color& color) override;

        // --- View / Camera ---

        /**
         * @brief Converts screen pixels (mouse pos) to world coordinates.
         * * Takes the current view (camera) into account.
         */
        engine::Vector2f screenToWorld(engine::Vector2f screenPos) override;
        
        void setViewCenter(engine::Vector2f center) override;
        engine::Vector2f getViewCenter() const override;
        void setViewSize(engine::Vector2f size) override;
        engine::Vector2u getWindowSize() const override;
        
        /// @brief Swaps buffers / displays the rendered frame.
        void endFrame() override;

        // --- SFML Specific Configuration ---

        /**
         * @brief Injects the resource manager.
         * * Required for `drawSprite` to work, as it needs to look up textures by ID.
         */
        void setResourceManager(std::shared_ptr<class IResourceManager> resourceManager);
        
        void setLogger(std::shared_ptr<ILogger> logger) { m_logger = logger; }
        
        /**
         * @brief Exposes the underlying SFML window.
         * * Use with caution. Primarily for GuiService (ImGui-SFML).
         */
        sf::RenderWindow& getNativeRenderWindow();

    private:
        sf::RenderWindow m_renderWindow;
        std::shared_ptr<class IResourceManager> m_resourceManager;
        
        // Cached shapes to avoid re-allocating them every frame for debug drawing
        sf::CircleShape m_circleShape;
        sf::RectangleShape m_rectShape;
        
        /// @brief Default font for debug text rendering.
        sf::Font m_font;
        
        std::shared_ptr<ILogger> m_logger;
    };
}