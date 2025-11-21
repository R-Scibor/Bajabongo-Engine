#include "engine/pch.h"
#include "EditorSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/rendering/IRenderer.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <box2d/box2d.h>
#include <SFML/Config.hpp>

namespace engine {

    EditorSystem::EditorSystem(std::shared_ptr<EngineContext> context)
        : m_context(std::move(context))
    {
        if (m_context->m_logManager) {
            m_logger = m_context->m_logManager->GetLogger("EditorSystem");
        }

        RegisterInspector<TransformComponent>([](entt::registry& reg, entt::entity e) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& transform = reg.get<TransformComponent>(e);
                bool changed = false; // Flaga czy coś zmieniliśmy

                float pos[2] = { transform.position.x, transform.position.y };
                if (ImGui::DragFloat2("Position", pos, 0.1f)) {
                    transform.position = { pos[0], pos[1] };
                    changed = true;
                }

                float scale[2] = { transform.scale.x, transform.scale.y };
                if (ImGui::DragFloat2("Scale", scale, 0.01f)) {
                    transform.scale = { scale[0], scale[1] };
                    // Skala nie wpływa na fizykę w Box2D (bez przebudowy kształtu),
                    // więc tu zmieniamy tylko grafikę.
                }

                float rotation = transform.rotation;
                if (ImGui::DragFloat("Rotation", &rotation, 1.0f)) {
                    transform.rotation = rotation;
                    changed = true;
                }
                
                // --- SYNC DO BOX2D (Teleportacja) ---
                if (changed && reg.all_of<PhysicsBodyComponent>(e)) {
                    auto& bodyComp = reg.get<PhysicsBodyComponent>(e);
                    if (b2Body_IsValid(bodyComp.bodyId)) {
                        // Konwersja Deg -> Rad
                        float angleRad = transform.rotation * (3.14159f / 180.0f);
                        b2Vec2 b2Pos = { transform.position.x, transform.position.y };
                        b2Rot b2Rotation = b2MakeRot(angleRad);

                        // Hard Reset pozycji w Box2D
                        b2Body_SetTransform(bodyComp.bodyId, b2Pos, b2Rotation);
                        
                        // Ważne: Obudź ciało, inaczej może zostać uśpione w starej pozycji
                        b2Body_SetAwake(bodyComp.bodyId, true);
                    }
                }
            }
        });

        RegisterInspector<RenderableComponent>([](entt::registry& reg, entt::entity e) {
            if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& renderable = reg.get<RenderableComponent>(e);
                
                ImGui::InputText("Texture ID", &renderable.spriteId);
                ImGui::InputInt("Layer", &renderable.layer);

                float color[4] = {
                    renderable.color.r / 255.0f,
                    renderable.color.g / 255.0f,
                    renderable.color.b / 255.0f,
                    renderable.color.a / 255.0f
                };
                if (ImGui::ColorEdit4("Color", color)) {
                    renderable.color.r = static_cast<unsigned char>(color[0] * 255);
                    renderable.color.g = static_cast<unsigned char>(color[1] * 255);
                    renderable.color.b = static_cast<unsigned char>(color[2] * 255);
                    renderable.color.a = static_cast<unsigned char>(color[3] * 255);
                }
            }
        });

        RegisterInspector<PhysicsBodyComponent>([](entt::registry& reg, entt::entity e) {
            if (ImGui::CollapsingHeader("Physics Body", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& body = reg.get<PhysicsBodyComponent>(e);
                ImGui::Text("Body ID: %d", body.bodyId.index1); // Displaying internal index as info
                
                // Since physics is managed by Box2D, modifying here might be complex without syncing.
                // We just display info for now.
                if (b2Body_IsValid(body.bodyId)) {
                     b2Vec2 pos = b2Body_GetPosition(body.bodyId);
                     ImGui::Text("B2 Pos: %.2f, %.2f", pos.x, pos.y);
                     float angle = b2Body_GetRotation(body.bodyId).c; // Approx
                     ImGui::Text("B2 Angle (cos): %.2f", angle);
                } else {
                    ImGui::TextColored({1, 0, 0, 1}, "Invalid Body ID");
                }
            }
        });
    }

    EditorSystem::~EditorSystem() = default;

    void EditorSystem::Update(float dt) {
        DrawHierarchy();
        DrawInspector();
        DrawSpawner();
        DrawSettings();

        // Logic for Placement Mode
        if (m_isPlacing && !m_placingArchetype.empty()) {
            // Left Click to Spawn (if mouse is not hovering ImGui window)
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().WantCaptureMouse) {
                auto mousePos = m_context->m_inputManager->getMousePosition();
                Vector2f spawnPos = { (float)mousePos.x, (float)mousePos.y };
                
                if (m_context->m_entityFactory) {
                    m_context->m_entityFactory->spawn(m_placingArchetype, spawnPos);
                    if (m_logger) m_logger->info("Spawned archetype '{}' at ({}, {})", m_placingArchetype, spawnPos.x, spawnPos.y);
                }
            }

            // Right Click to Cancel
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                m_isPlacing = false;
                m_placingArchetype.clear();
                if (m_logger) m_logger->info("Placement mode cancelled.");
            }
        }
    }

    void EditorSystem::Render() {
        // Visual Selection Indicator
        if (m_selectedEntity != entt::null && m_context->m_registry->valid(m_selectedEntity)) {
            if (m_context->m_registry->all_of<TransformComponent>(m_selectedEntity)) {
                auto& transform = m_context->m_registry->get<TransformComponent>(m_selectedEntity);
                
                // Simple visual size estimation
                // We don't have easy access to sprite size without potential compilation issues or complexity,
                // so we'll just use a reasonable default scaled by the transform.
                float baseSize = 25.0f;
                float maxScale = std::max(std::abs(transform.scale.x), std::abs(transform.scale.y));
                float radius = baseSize * maxScale;

                // Draw a selection circle
                if (m_context->m_renderer) {
                    m_context->m_renderer->drawCircle(transform.position, radius);
                }
            }
        }
    }

    void EditorSystem::DrawHierarchy() {
        ImGui::Begin("Hierarchy");

        auto& registry = *m_context->m_registry;
        
        // Use a list of entities to avoid iterator invalidation during deletion
        std::vector<entt::entity> entities;
        entities.reserve(registry.storage<entt::entity>().size());
        for(auto entity : registry.storage<entt::entity>()) {
            entities.push_back(entity);
        }

        for (auto entity : entities) {
            if (!registry.valid(entity)) continue;

            std::string label = "Entity " + std::to_string(static_cast<uint32_t>(entity));
            
            // Use MetaComponent name if available
            if (registry.all_of<MetaComponent>(entity)) {
                label = registry.get<MetaComponent>(entity).name + " (" + std::to_string(static_cast<uint32_t>(entity)) + ")";
            }

            ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", label.c_str());
            if (ImGui::IsItemClicked()) {
                m_selectedEntity = entity;
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete")) {
                    registry.destroy(entity);
                    if (m_selectedEntity == entity) {
                        m_selectedEntity = entt::null;
                    }
                }
                ImGui::EndPopup();
            }

            if (opened) {
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    void EditorSystem::DrawInspector() {
        ImGui::Begin("Inspector");

        if (m_selectedEntity != entt::null && m_context->m_registry->valid(m_selectedEntity)) {
            auto& registry = *m_context->m_registry;
            
            ImGui::Text("Entity ID: %d", static_cast<uint32_t>(m_selectedEntity));
            
            if (registry.all_of<MetaComponent>(m_selectedEntity)) {
                auto& meta = registry.get<MetaComponent>(m_selectedEntity);
                ImGui::InputText("Name", &meta.name);
            }

            ImGui::Separator();

            for (auto& [type, inspector] : m_inspectors) {
                inspector(registry, m_selectedEntity);
            }

            ImGui::Separator();
            ImGui::Spacing();
            
            if (ImGui::Button("Destroy Entity", ImVec2(-1, 0))) {
                registry.destroy(m_selectedEntity);
                m_selectedEntity = entt::null;
            }

        } else {
            ImGui::Text("No entity selected.");
        }

        ImGui::End();
    }

    void EditorSystem::DrawSpawner() {
        ImGui::Begin("Asset Browser");

        if (ImGui::CollapsingHeader("Archetypes", ImGuiTreeNodeFlags_DefaultOpen)) {
            static int selectedIndex = -1;
            static std::string selectedArchetype;
            
            if (m_context->m_archetypeManager) {
                auto& archetypes = m_context->m_archetypeManager->getAllArchetypes();
                
                // Lista wyboru
                if (ImGui::BeginListBox("##archetypes")) {
                    int i = 0;
                    for (const auto& [name, data] : archetypes) {
                        const bool is_selected = (selectedIndex == i);
                        if (ImGui::Selectable(name.c_str(), is_selected)) {
                            selectedIndex = i;
                            selectedArchetype = name;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                        i++;
                    }
                    ImGui::EndListBox();
                }

                // Przycisk Spawn Mode
                ImGui::Spacing();
                if (!selectedArchetype.empty()) {
                    bool isCurrent = (m_isPlacing && m_placingArchetype == selectedArchetype);
                    
                    std::string btnLabel = isCurrent ? "Stop Placing" : "Start Placing";
                    
                    if (ImGui::Button(btnLabel.c_str(), ImVec2(-1, 0))) {
                        if (isCurrent) {
                            m_isPlacing = false;
                            m_placingArchetype.clear();
                        } else {
                            m_isPlacing = true;
                            m_placingArchetype = selectedArchetype;
                            if (m_logger) m_logger->info("Placement mode started for '{}'", selectedArchetype);
                        }
                    }
                    
                    if (m_isPlacing) {
                        ImGui::TextColored({0, 1, 0, 1}, "Placement Active! Left click to spawn, Right click to cancel.");
                    }
                }
            }
        }
        ImGui::End();
    }

    void EditorSystem::DrawSettings() {
        ImGui::Begin("Debug Settings");
        ImGui::Checkbox("Show Physics Wireframe", &m_context->debugFlags.showPhysics);
        ImGui::Checkbox("Show Hitboxes", &m_context->debugFlags.showHitboxes);
        ImGui::Checkbox("Pause Simulation", &m_context->debugFlags.pauseGame);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
    }

} // namespace engine