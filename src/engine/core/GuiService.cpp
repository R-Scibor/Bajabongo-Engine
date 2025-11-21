#include "engine/pch.h"
#include "GuiService.hpp"
#include "engine/core/ILogger.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

namespace engine {

    GuiService::GuiService() = default;

    GuiService::~GuiService() {
        Shutdown();
    }

    void GuiService::SetLogger(std::shared_ptr<ILogger> logger) {
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
        
        ImGuiIO& io = ImGui::GetIO();
        // Docking might not be enabled in this build of ImGui
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        m_initialized = true;
        if (m_logger) m_logger->info("GuiService: Initialized successfully.");
    }

    void GuiService::Shutdown() {
        if (m_initialized) {
            ImGui::SFML::Shutdown();
            m_initialized = false;
            if (m_logger) m_logger->info("GuiService: Shut down.");
        }
    }

    bool GuiService::HandleEvent(const sf::Window& window, const sf::Event& event) {
        if (!m_initialized) return false;

        ImGui::SFML::ProcessEvent(window, event);

        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
            return true;
        }
        return false;
    }

    void GuiService::BeginFrame(sf::RenderWindow& window, const sf::Time& dt) {
        if (!m_initialized) return;
        ImGui::SFML::Update(window, dt);
    }

    void GuiService::Render(sf::RenderWindow& window) {
        if (!m_initialized) return;
        ImGui::SFML::Render(window);
    }

} // namespace engine