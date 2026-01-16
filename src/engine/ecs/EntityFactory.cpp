#include "engine/pch.h"
#include "EntityFactory.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/ParentComponent.hpp"
#include "engine/components/ChildComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/CameraFocusComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <SFML/Graphics/Color.hpp>

namespace engine {

    namespace ComponentNames {
        static constexpr const char* Transform = "Transform";
        static constexpr const char* Renderable = "Renderable";
        static constexpr const char* Physics = "Physics";
        static constexpr const char* Animation = "Animation";
        static constexpr const char* AnimationState = "AnimationState";
        static constexpr const char* CameraFocus = "CameraFocus";
    }

    EntityFactory::EntityFactory(EngineContext& context, std::shared_ptr<ArchetypeManager> archetypeManager)
        : m_context(context), m_archetypeManager(archetypeManager) {
        if (m_context.m_logManager) {
            m_logger = m_context.m_logManager->GetLogger("EntityFactory");
        }
        registerDefaultLoaders();
    }

    void EntityFactory::registerComponentLoader(const std::string& componentName, ComponentLoaderFn loader) {
        m_componentRegistry[componentName] = loader;
    }

    entt::entity EntityFactory::spawn(const std::string& archetypeId, const Vector2f& position) {
        const auto* archetypeData = m_archetypeManager->getArchetype(archetypeId);
        if (!archetypeData) {
            if (m_logger) m_logger->error("EntityFactory: Archetype '{}' not found", archetypeId);
            return entt::null;
        }

        auto entity = m_context.m_registry->create();

        // 1. Initialize TransformComponent with spawn position (crucial for other components)
        m_context.m_registry->emplace<TransformComponent>(entity, position, 0.f, Vector2f{1.f, 1.f});

        // 2. Add MetaComponent with archetype name
        m_context.m_registry->emplace<MetaComponent>(entity, archetypeId);

        // 3. Iterate over keys in JSON and call registered loaders
        for (auto& [key, value] : archetypeData->items()) {
            // Special case: 'children' is handled separately after this loop (recursive spawning)
            if (key == "children") continue;

            auto it = m_componentRegistry.find(key);
            if (it != m_componentRegistry.end()) {
                try {
                    it->second(*m_context.m_registry, entity, value);
                } catch (const std::exception& e) {
                    if (m_logger) m_logger->error("EntityFactory: Error loading component '{}' for archetype '{}': {}", key, archetypeId, e.what());
                }
            } else {
                if (m_logger) m_logger->warn("EntityFactory: No loader registered for component '{}'", key);
            }
        }

        // 4. Handle Children
        if (archetypeData->contains("children") && (*archetypeData)["children"].is_array()) {
            
            // Ensure parent has a ChildComponent to track its kids
            auto& childComp = m_context.m_registry->get_or_emplace<ChildComponent>(entity);

            for (const auto& childDef : (*archetypeData)["children"]) {
                std::string childArchetype = childDef.value("archetype", "");
                if (childArchetype.empty()) continue;

                // RECURSION: Spawn the child at (0,0) initially.
                // The HierarchySystem will move it to the correct spot next frame.
                // Note: We might want to pass an offset or a flag to prevent Transform init if we cared about perf,
                // but (0,0) is fine as it gets overwritten by hierarchy.
                entt::entity childEntity = spawn(childArchetype, {0.f, 0.f});

                // Read Local Offsets
                Vector2f localPos = {0.f, 0.f};
                if (childDef.contains("offset") && childDef["offset"].is_array()) {
                    localPos.x = childDef["offset"][0];
                    localPos.y = childDef["offset"][1];
                }
                float localRot = childDef.value("rotation", 0.f);

                // AUTO-LINKING: Add ParentComponent to child
                m_context.m_registry->emplace<ParentComponent>(
                    childEntity,
                    entity, // Parent ID
                    localPos,
                    localRot
                );

                // Add to parent's ChildComponent list (for destruction logic later)
                childComp.children.push_back(childEntity);
            }
        }

        return entity;
    }

    void EntityFactory::registerDefaultLoaders() {
        // --- Transform ---
        registerComponentLoader(ComponentNames::Transform, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            auto& transform = registry.get<TransformComponent>(entity);
            
            if (data.contains("rotation")) {
                transform.rotation = data["rotation"];
            }
            if (data.contains("scale") && data["scale"].is_array()) {
                transform.scale.x = data["scale"][0];
                transform.scale.y = data["scale"][1];
            }
        });

        // --- Renderable ---
        registerComponentLoader(ComponentNames::Renderable, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            std::string spriteId = data.value("spriteId", "");
            int layer = data.value("layer", 0);
            sf::Color color = sf::Color::White;

            if (data.contains("color") && data["color"].is_array()) {
                auto& c = data["color"];
                if (c.size() >= 3) {
                    color.r = c[0].get<int>();
                    color.g = c[1].get<int>();
                    color.b = c[2].get<int>();
                    color.a = c.size() > 3 ? c[3].get<int>() : 255;
                }
            }

            if (!spriteId.empty()) {
                registry.emplace<RenderableComponent>(entity, spriteId, layer, color);
            }
        });

        // --- Physics ---
        registerComponentLoader(ComponentNames::Physics, [this](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            Vector2f position = {0.f, 0.f};
            if (registry.all_of<TransformComponent>(entity)) {
                position = registry.get<TransformComponent>(entity).position;
            }

            Vector2f size = {0.f, 0.f};
            if (data.contains("size") && data["size"].is_array()) {
                size.x = data["size"][0];
                size.y = data["size"][1];
            }

            // Validation for size
            if (size.x <= 0.001f || size.y <= 0.001f) {
                if (m_logger) m_logger->warn("EntityFactory: Physics component has invalid size for entity {}", static_cast<uint32_t>(entity));
                size = {32.f, 32.f}; // Fallback
            }

            bool isStatic = (data.value("type", "static") == "static");
            float density = data.value("density", 1.0f);
            bool isSensor = data.value("isSensor", false);
            bool fixedRotation = data.value("fixedRotation", false);
            float linearDamping = data.value("linearDamping", 0.0f);

            // Default categories
            unsigned int categoryBits = PhysicsCategory::Default;
            unsigned int maskBits = PhysicsCategory::All;

            if (data.contains("category")) {
                std::string cat = data["category"];
                if (cat == "Player") categoryBits = PhysicsCategory::Player;
                else if (cat == "Enemy") categoryBits = PhysicsCategory::Enemy;
                else if (cat == "Wall") categoryBits = PhysicsCategory::Wall;
                else if (cat == "Projectile") categoryBits = PhysicsCategory::Projectile;
                else if (cat == "LowObstacle") categoryBits = PhysicsCategory::LowObstacle;
                else if (cat == "Sensor") categoryBits = PhysicsCategory::Sensor;
            }

            // [Modified] Visibility Blocker Logic
            // Default to NOT blocking vision unless explicitly set to true
            // This ensures players/enemies don't block vision by default.
            bool blocksVision = data.value("blocksVision", false);
            if (blocksVision) {
                categoryBits |= PhysicsCategory::VisibilityBlocker;
            } else {
                categoryBits &= ~PhysicsCategory::VisibilityBlocker;
            }

            // Custom logic for Players and Enemies to ensure they collide with correct things
            if (categoryBits == PhysicsCategory::Player) {
                 maskBits = PhysicsCategory::Default | PhysicsCategory::Enemy | PhysicsCategory::Wall | PhysicsCategory::LowObstacle | PhysicsCategory::Projectile;
            } else if (categoryBits == PhysicsCategory::Enemy) {
                 maskBits = PhysicsCategory::Default | PhysicsCategory::Player | PhysicsCategory::Wall | PhysicsCategory::LowObstacle | PhysicsCategory::Projectile;
            }

            PendingPhysicsBodyComponent pending{
                .position = position,
                .isStatic = isStatic,
                .fixedRotation = fixedRotation,
                .linearDamping = linearDamping,
                .isBullet = false,
                .initialVelocity = {0.0f, 0.0f},
                .rotation = 0.0f
            };

            // Parse fixtures
            // Check if we have multiple fixtures defined in JSON
            if (data.contains("fixtures") && data["fixtures"].is_array()) {
                for (const auto& fixData : data["fixtures"]) {
                     FixtureDef fixDef;
                     if (fixData.contains("size") && fixData["size"].is_array()) {
                        fixDef.size.x = fixData["size"][0];
                        fixDef.size.y = fixData["size"][1];
                     }
                     if (fixData.contains("offset") && fixData["offset"].is_array()) {
                        fixDef.offset.x = fixData["offset"][0];
                        fixDef.offset.y = fixData["offset"][1];
                     }
                     fixDef.density = fixData.value("density", 1.0f);
                     fixDef.isSensor = fixData.value("isSensor", false);
                     
                     // Category/Mask parsing for specific fixtures (if needed)
                     // For now, let's implement basic string-to-category mapping locally or helper
                     if (fixData.contains("category")) {
                        std::string c = fixData["category"];
                        if (c == "Player") fixDef.categoryBits = PhysicsCategory::Player;
                        else if (c == "PlayerHurtbox") fixDef.categoryBits = PhysicsCategory::Hurtbox; // Use Hurtbox category
                        else if (c == "Hurtbox") fixDef.categoryBits = PhysicsCategory::Hurtbox;
                        else if (c == "Enemy") fixDef.categoryBits = PhysicsCategory::Enemy;
                        else if (c == "Wall") fixDef.categoryBits = PhysicsCategory::Wall;
                        else if (c == "Projectile") fixDef.categoryBits = PhysicsCategory::Projectile;
                        else if (c == "LowObstacle") fixDef.categoryBits = PhysicsCategory::LowObstacle;
                        else if (c == "Sensor") fixDef.categoryBits = PhysicsCategory::Sensor;
                     } else {
                         fixDef.categoryBits = categoryBits; // Inherit from body default
                     }

                     // Apply blocksVision logic to individual fixtures too
                     // Note: If inherit from body default, it's already applied. 
                     // If custom category was set above, we need to apply it again unless overriden by specific fixture property.
                     bool fixtureBlocksVision = fixData.value("blocksVision", blocksVision);
                     if (fixtureBlocksVision) {
                         fixDef.categoryBits |= PhysicsCategory::VisibilityBlocker;
                     } else {
                         fixDef.categoryBits &= ~PhysicsCategory::VisibilityBlocker;
                     }
                     
                     // Mask parsing could be complex list of strings...
                     // For MVP, if it's Player Feet, use standard Player mask
                     // If it's Hurtbox, use Projectile mask
                     if (fixDef.categoryBits == PhysicsCategory::Player) {
                         fixDef.maskBits = PhysicsCategory::Default | PhysicsCategory::Enemy | PhysicsCategory::Wall | PhysicsCategory::LowObstacle;
                     } else if (fixDef.categoryBits == PhysicsCategory::Hurtbox) {
                         fixDef.maskBits = PhysicsCategory::Projectile;
                     } else if (fixDef.categoryBits == PhysicsCategory::Enemy) {
                         fixDef.maskBits = PhysicsCategory::Default | PhysicsCategory::Player | PhysicsCategory::Wall | PhysicsCategory::LowObstacle | PhysicsCategory::Projectile;
                     } else {
                         fixDef.maskBits = maskBits; // Inherit
                     }
                     
                     pending.fixtures.push_back(fixDef);
                }
            } else {
                // Backward compatibility: Create single fixture from body params
                FixtureDef fixDef;
                fixDef.size = size;
                fixDef.density = density;
                fixDef.isSensor = isSensor;
                fixDef.categoryBits = categoryBits;
                fixDef.maskBits = maskBits;
                pending.fixtures.push_back(fixDef);
            }

            registry.emplace<PendingPhysicsBodyComponent>(entity, pending);
        });

        // --- Animation ---
        registerComponentLoader(ComponentNames::Animation, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            AnimationComponent animComp;
            animComp.currentClipId = data.value("defaultClip", "");
            animComp.isPlaying = data.value("playing", false);
            
            registry.emplace<AnimationComponent>(entity, animComp);
        });

        // --- AnimationState ---
        registerComponentLoader(ComponentNames::AnimationState, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            AnimationStateComponent stateComp;
            
            std::string initialState = data.value("initialState", "Idle");
            if (initialState == "Walk") {
                stateComp.state = AnimationState::Walk;
            } else {
                stateComp.state = AnimationState::Idle;
            }
            
            std::string initialFacing = data.value("initialFacing", "Right");
            if (initialFacing == "Left") {
                stateComp.facing = FacingDirection::Left;
            } else {
                stateComp.facing = FacingDirection::Right;
            }
            
            registry.emplace<AnimationStateComponent>(entity, stateComp);
        });

        // --- CameraFocus ---
        registerComponentLoader(ComponentNames::CameraFocus, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            CameraFocusComponent cameraComp;
            cameraComp.viewHeight = data.value("viewHeight", 1080.0f);
            cameraComp.smoothness = data.value("smoothness", 0.1f);
            
            registry.emplace<CameraFocusComponent>(entity, cameraComp);
        });
    }

} // namespace engine