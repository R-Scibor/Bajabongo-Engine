#include "engine/pch.h"
#include "HudSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/GuiService.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include <entt/entt.hpp>
#include <imgui.h>

namespace game {

    HudSystem::HudSystem(engine::EngineContext& context)
        : m_context(context)
    {
    }

    void HudSystem::render()
    {
        auto& registry = *m_context.m_registry;

        // Find the player entity (assuming single player for now)
        auto view = registry.view<PlayerComponent, WeaponComponent>();
        
        // We only expect one player
        view.each([&](entt::entity entity, const PlayerComponent& player, const WeaponComponent& weapon) {
            auto* health = registry.try_get<HealthComponent>(entity);
            // Setup ImGui window for HUD
            // Position: Bottom Right
            ImGuiIO& io = ImGui::GetIO();
            float windowWidth = 260.0f;
            float windowHeight = 140.0f;
            float padding = 20.0f;
            
            ImGui::SetNextWindowPos(
                ImVec2(io.DisplaySize.x - windowWidth - padding, io.DisplaySize.y - windowHeight - padding), 
                ImGuiCond_Always
            );
            ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);

            // Transparent background, no title bar, no resize, etc.
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
                                           ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | 
                                           ImGuiWindowFlags_NoInputs;

            // ImGui 1.92+: PushFont(font, size) rasterizes at that size. nullptr
            // keeps the current font (AddFontDefault fallback if the TTF failed).
            ImFont* hudFont = m_context.m_guiService ? m_context.m_guiService->GetHudFont() : nullptr;
            const float ammoSize = 32.0f;
            const float bodySize = 16.0f;

            if (ImGui::Begin("HUD_Ammo", nullptr, windowFlags)) {
                
                if (weapon.isReloading) {
                    ImGui::PushFont(hudFont, ammoSize);
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RELOADING...");
                    ImGui::PopFont();

                    ImGui::PushFont(hudFont, bodySize);
                    float progress = 1.0f - (weapon.reloadTimer / weapon.reloadDuration);
                    ImGui::ProgressBar(progress, ImVec2(-1, 0), "");
                    ImGui::PopFont();
                    
                } else {
                    ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    if (weapon.currentAmmo == 0) {
                        textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                    } else if (weapon.currentAmmo < weapon.magSize * 0.3f) {
                        textColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
                    }

                    ImGui::PushFont(hudFont, ammoSize);
                    ImGui::TextColored(textColor, "%d / %d", weapon.currentAmmo, weapon.totalAmmo);
                    ImGui::PopFont();

                    ImGui::PushFont(hudFont, bodySize);
                    ImGui::Text("Magazine: %d", weapon.magSize);
                    ImGui::PopFont();
                }

                ImGui::PushFont(hudFont, bodySize);
                ImGui::Separator();
                ImGui::Text("Medkits: %d/%d", player.medkits, player.maxMedkits);
                if (health) {
                    ImGui::Text("HP: %.0f/%.0f", health->currentHp, health->maxHp);
                }
                ImGui::PopFont();
            }
            ImGui::End();
            
            // Break after finding the first player (if multiple players exist, we'd need a different logic)
        });
    }

} // namespace game