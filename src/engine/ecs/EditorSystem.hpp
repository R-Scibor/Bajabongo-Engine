#pragma once

#include <entt/entt.hpp>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace engine {

    struct EngineContext;
    class ILogger;

    class EditorSystem {
    public:
        using InspectorFunc = std::function<void(entt::registry&, entt::entity)>;

        explicit EditorSystem(std::shared_ptr<EngineContext> context);
        ~EditorSystem();

        void Update(float dt);
        void Render(); // Can be used if we want to separate update/render logic for UI

        template <typename T>
        void RegisterInspector(InspectorFunc inspector) {
            m_inspectors[std::type_index(typeid(T))] = [inspector](entt::registry& reg, entt::entity e) {
                if (reg.all_of<T>(e)) {
                    inspector(reg, e);
                }
            };
        }

    private:
        void DrawHierarchy();
        void DrawInspector();

        std::shared_ptr<EngineContext> m_context;
        std::shared_ptr<ILogger> m_logger;
        entt::entity m_selectedEntity = entt::null;
        
        std::unordered_map<std::type_index, InspectorFunc> m_inspectors;
    };

} // namespace engine