#pragma once

#include <entt/entt.hpp>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace engine {

    struct EngineContext;
    class ILogger;

    /**
     * @class EditorSystem
     * @brief Manages the in-game editor interface using ImGui.
     *
     * The EditorSystem provides tools for inspecting entities, modifying components,
     * spawning new entities from archetypes, and visualizing the entity hierarchy.
     * It handles user input for selecting and placing entities within the game world.
     */
    class EditorSystem {
    public:
        /** @brief Function signature for component inspectors. */
        using InspectorFunc = std::function<void(entt::registry&, entt::entity)>;

        /**
         * @brief Constructs a new EditorSystem.
         *
         * @param context Shared pointer to the engine context.
         */
        explicit EditorSystem(std::shared_ptr<EngineContext> context);

        /** @brief Destructor for EditorSystem. */
        ~EditorSystem();

        /**
         * @brief Updates the editor system.
         *
         * Handles input processing, entity selection, and drawing the editor UI.
         *
         * @param dt Delta time since the last frame.
         */
        void Update(float dt);

        /**
         * @brief Renders editor-specific visuals.
         *
         * Draws selection indicators, ghost previews for placement, and other
         * visual aids that are not part of the standard game rendering.
         */
        void Render();

        /**
         * @brief Registers an inspector function for a specific component type.
         *
         * This allows custom UI to be drawn for components in the Inspector window.
         *
         * @tparam T The component type to inspect.
         * @param inspector The function to call when inspecting a component of type T.
         */
        template <typename T>
        void RegisterInspector(InspectorFunc inspector) {
            m_inspectors[std::type_index(typeid(T))] = [inspector](entt::registry& reg, entt::entity e) {
                if (reg.all_of<T>(e)) {
                    inspector(reg, e);
                }
            };
        }

    private:
        /** @brief Draws the entity hierarchy window. */
        void DrawHierarchy();
        
        /** @brief Draws the inspector window for the selected entity. */
        void DrawInspector();
        
        /** @brief Draws the asset browser and spawner window. */
        void DrawSpawner();
        
        /** @brief Draws the debug settings window. */
        void DrawSettings();

        /** @brief Shared pointer to the engine context. */
        std::shared_ptr<EngineContext> m_context;
        
        /** @brief Logger instance for the EditorSystem. */
        std::shared_ptr<ILogger> m_logger;
        
        /** @brief Currently selected entity in the editor. */
        entt::entity m_selectedEntity = entt::null;
        
        // Spawning State
        /** @brief Flag indicating if the editor is in placement mode. */
        bool m_isPlacing = false;
        
        /** @brief ID of the archetype currently being placed. */
        std::string m_placingArchetype;

        /** @brief Map of registered component inspectors. */
        std::unordered_map<std::type_index, InspectorFunc> m_inspectors;
    };

} // namespace engine