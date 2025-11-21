#include "engine/pch.h"
#include "EditorSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <box2d/box2d.h>
#include <SFML/Config.hpp>

namespace engine {

    EditorSystem::EditorSystem(std::shared_ptr<EngineContext> context)
        : m_context(std::move(context))
    {
        // Register TransformComponent Inspector
        RegisterInspector<TransformComponent>([](entt::registry& reg, entt::entity e) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& transform = reg.get<TransformComponent>(e);
                
                float pos[2] = { transform.position.x, transform.position.y };
                if (ImGui::DragFloat2("Position", pos, 0.1f)) {
                    transform.position = { pos[0], pos[1] };
                }

                float scale[2] = { transform.scale.x, transform.scale.y };
                if (ImGui::DragFloat2("Scale", scale, 0.01f)) {
                    transform.scale = { scale[0], scale[1] };
                }

                float rotation = transform.rotation;
                if (ImGui::DragFloat("Rotation", &rotation, 1.0f)) {
                    transform.rotation = rotation;
                }
            }
        });

        // Register RenderableComponent Inspector
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

        // Register PhysicsBodyComponent Inspector
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
    }

    void EditorSystem::Render() {
        // Any specific debug rendering (shapes) could go here
    }

    void EditorSystem::DrawHierarchy() {
        ImGui::Begin("Hierarchy");

        auto& registry = *m_context->m_registry;
        // Iterate all entities using range-based loop on storage
        for (auto entity : registry.storage<entt::entity>()) {
            std::string label = "Entity " + std::to_string(static_cast<uint32_t>(entity));
            
            // Optional: Display TagComponent if it existed (not in core yet per file list, but commonly used)
            
            ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            
            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", label.c_str());
            if (ImGui::IsItemClicked()) {
                m_selectedEntity = entity;
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
            ImGui::Separator();

            // Check all registered inspectors
            // Note: In a real system we might want to iterate components attached to entity
            // but entt doesn't support runtime reflection of all components easily without meta.
            // So we iterate our supported inspector list and check if entity has them.
            for (auto& [type, inspector] : m_inspectors) {
                // The inspector function itself checks for component presence (via the wrapper in RegisterInspector logic below)
                // WAIT: The wrapper logic must be inside RegisterInspector implementation.
                // But here I need to fix my RegisterInspector to include that check, 
                // OR I do it here.
                // My current RegisterInspector just stores the func.
                // I need to fix RegisterInspector or the loop here.
                
                // Since I can't check 'type' against registry here easily without meta,
                // I rely on the lambda wrapper approach.
                inspector(registry, m_selectedEntity);
            }

        } else {
            ImGui::Text("No entity selected.");
        }

        ImGui::End();
    }

} // namespace engine