#include "engine/pch.h"
#include "GuiService.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

namespace engine {

    GuiService::GuiService() = default;

    GuiService::~GuiService() {
        Shutdown();
    }

    void GuiService::Init(sf::RenderWindow& window) {
        if (m_initialized) return;

        if (!ImGui::SFML::Init(window)) {
            // Handle error? For now just return or log if we had logger
            return;
        }
        
        ImGuiIO& io = ImGui::GetIO();
        // Docking might not be enabled in this build of ImGui
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        m_initialized = true;
    }

    void GuiService::Shutdown() {
        if (m_initialized) {
            ImGui::SFML::Shutdown();
            m_initialized = false;
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