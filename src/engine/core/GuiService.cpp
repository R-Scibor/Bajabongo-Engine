#include "engine/pch.h"
#include "GuiService.hpp"
#include "engine/core/ILogger.hpp"

#include <imgui.h>
#include <imgui-SFML.h>
#include <fstream>

namespace engine {

    GuiService::GuiService() = default;

    GuiService::~GuiService() {
        Shutdown();
    }

    void GuiService::setLogger(std::shared_ptr<ILogger> logger) {
        m_logger = std::move(logger);
    }

    void GuiService::Init(sf::RenderWindow& window) {
        if (m_initialized) {
            if (m_logger) m_logger->warn("GuiService: Already initialized.");
            return;
        }

        if (!ImGui::SFML::Init(window)) {
            if (m_logger) m_logger->error("GuiService: Failed to initialize ImGui-SFML.");
            return;
        }

        // imgui-sfml 3.0 (vcpkg patch for imgui 1.92+) dropped the Init path that
        // used to build the default font atlas via GetTexDataAsRGBA32. The
        // loadDefaultFont argument is now unused, so the atlas is empty and
        // ImGui::SFML::Update asserts io.Fonts->Fonts.Size > 0. The texture
        // itself is uploaded later by the RendererHasTextures backend.
        const ImGuiIO& io = ImGui::GetIO();
        // Editor / debug UI keeps the baked-in ProggyClean font. The HUD loads
        // Bebas Neue separately and selects it with ImGui::PushFont(font, size)
        // (ImGui 1.92+ API). SetWindowFontScale is obsolete and, with dynamic
        // fonts, rasterizes extra sizes into the atlas.
        if (io.Fonts->Fonts.Size == 0) {
            io.Fonts->AddFontDefault();
        }

        // Same path convention as SFMLRenderer / runtime copy: the exe lives in
        // out/game/$<CONFIG>/, assets are at ../../assets (or ./assets).
        const char* fontPaths[] = {
            "assets/fonts/default.ttf",
            "../../assets/fonts/default.ttf",
        };
        m_hudFont = nullptr;
        for (const char* path : fontPaths) {
            // Probe first: AddFontFromFileTTF asserts in Debug if the file is missing,
            // and we try ./assets before ../../assets to match SFMLRenderer.
            if (!std::ifstream(path, std::ios::binary)) continue;
            if (ImFont* font = io.Fonts->AddFontFromFileTTF(path, 16.0f)) {
                m_hudFont = font;
                if (m_logger) m_logger->info("GuiService: Loaded HUD font from {}.", path);
                break;
            }
        }
        if (!m_hudFont && m_logger) {
            m_logger->warn("GuiService: Could not load assets/fonts/default.ttf; HUD will use the default ImGui font.");
        }

        m_initialized = true;
        if (m_logger) m_logger->info("GuiService: Initialized successfully.");
    }

    void GuiService::Shutdown() {
        if (m_initialized) {
            ImGui::SFML::Shutdown();
            m_hudFont = nullptr;
            m_initialized = false;
            if (m_logger) m_logger->info("GuiService: Shut down.");
        }
    }

    bool GuiService::HandleEvent(const sf::Window& window, const sf::Event& event) {
        if (!m_initialized) return false;

        // Feed the event to ImGui (updates key states, mouse pos, etc.)
        ImGui::SFML::ProcessEvent(window, event);

        // Check if ImGui wants to capture inputs
        ImGuiIO& io = ImGui::GetIO();

        // WantCaptureMouse: True if mouse is hovering an ImGui window
        // WantCaptureKeyboard: True if an input field is active
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
            return true; // Input consumed by UI
        }

        return false; // Input available for Game Logic
    }

    void GuiService::BeginFrame(sf::RenderWindow& window, const sf::Time& dt) {
        if (!m_initialized) return;
        // Updates internal ImGui timers and input state
        ImGui::SFML::Update(window, dt);
    }

    void GuiService::Render(sf::RenderWindow& window) {
        if (!m_initialized) return;
        // Generates draw lists and renders them via SFML
        ImGui::SFML::Render(window);
    }

} // namespace engine
