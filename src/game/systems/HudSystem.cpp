#include "engine/pch.h"
#include "HudSystem.hpp"
#include "engine/core/EngineContext.hpp"
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
            float windowWidth = 200.0f;
            float windowHeight = 100.0f;
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

            if (ImGui::Begin("HUD_Ammo", nullptr, windowFlags)) {
                
                ImGui::SetWindowFontScale(2.0f); // Make text larger
                
                if (weapon.isReloading) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RELOADING...");
                    
                    // Progress bar for reload
                    float progress = 1.0f - (weapon.reloadTimer / weapon.reloadDuration);
                    ImGui::ProgressBar(progress, ImVec2(-1, 0), "");
                    
                } else {
                    // Ammo Count: Current / Total
                    // Using different colors for low ammo?
                    ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    if (weapon.currentAmmo == 0) {
                        textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                    } else if (weapon.currentAmmo < weapon.magSize * 0.3f) {
                        textColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
                    }

                    ImGui::TextColored(textColor, "%d / %d", weapon.currentAmmo, weapon.totalAmmo);
                    
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::Text("Magazine: %d", weapon.magSize);
                }

                ImGui::Separator();
                ImGui::Text("Medkits: %d/%d", player.medkits, player.maxMedkits);
                if (health) {
                    ImGui::Text("HP: %.0f/%.0f", health->currentHp, health->maxHp);
                }
            }
            ImGui::End();
            
            // Break after finding the first player (if multiple players exist, we'd need a different logic)
        });
    }

} // namespace game