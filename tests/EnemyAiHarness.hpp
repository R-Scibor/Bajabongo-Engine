#pragma once

#include "HeadlessScene.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/ecs/HierarchySystem.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsEventSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/systems/LifetimeSystem.hpp"
#include "game/ai/EnemyCombatSystem.hpp"
#include "game/ai/EnemyDetectionSystem.hpp"
#include "game/ai/EnemyMovementSystem.hpp"
#include "game/ai/EnemyStateTransitions.hpp"
#include "game/ai/EnemyStuckRecoverySystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/setup/GameComponentRegistry.hpp"
#include "game/systems/VisibilitySystem.hpp"

#include <box2d/box2d.h>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace baja::test {

    struct EnemyAiRunStats {
        int enemyCount = 0;
        int stuckChecks = 0;
        int movedZero = 0;
        int teleports = 0;
        int impulses = 0;
        int visibleImpulses = 0;
        int recovered = 0;
        int killed = 0;
        int barelyMoved = 0;
        int maxStuck = 0;
        float minDisp = 0.0f;
        float maxDisp = 0.0f;
        std::string error;
        std::vector<std::string> stuckLog;
    };

    class CapturingLogger final : public engine::ILogger {
    public:
        explicit CapturingLogger(std::vector<std::string>& lines) : m_lines(lines) {}

        void log(spdlog::level::level_enum level, std::string_view message) override {
            if (level < spdlog::level::info) return;
            m_lines.emplace_back(message);
        }

    private:
        std::vector<std::string>& m_lines;
    };

    class CapturingLogManager final : public engine::ILoggerManager {
    public:
        explicit CapturingLogManager(std::vector<std::string>& lines)
            : m_logger(std::make_shared<CapturingLogger>(lines)) {}

        std::shared_ptr<engine::ILogger> getLogger(const std::string&) override { return m_logger; }

    private:
        std::shared_ptr<engine::ILogger> m_logger;
    };

    inline int countSubstring(const std::vector<std::string>& lines, std::string_view needle) {
        int count = 0;
        for (const auto& line : lines) {
            if (line.find(needle) != std::string::npos) ++count;
        }
        return count;
    }

    /// Headless hospital/hideout AI run used for the pre-fix baseline (seed 1, 1800 frames).
    inline EnemyAiRunStats runHeadlessEnemyAi(const std::filesystem::path& assets, const std::string& mapName,
                                              std::uint32_t seed, int frames, float dt = 1.0f / 60.0f) {
        EnemyAiRunStats stats;
        const auto mapPath = assets / "data" / ("map_" + mapName + ".json");
        const auto archetypesPath = assets / "data" / "archetypes.json";
        if (!std::filesystem::is_regular_file(mapPath) || !std::filesystem::is_regular_file(archetypesPath)) {
            stats.error = "map or archetypes.json is not available";
            return stats;
        }

        HeadlessScene scene;
        auto logManager = std::make_shared<CapturingLogManager>(stats.stuckLog);
        scene.context.m_logManager = logManager;
        scene.context.m_random.seed(seed);

        auto archetypes = std::make_shared<engine::ArchetypeManager>(logManager);
        if (!archetypes->loadArchetypes(archetypesPath.string())) {
            stats.error = "failed to load archetypes";
            return stats;
        }
        auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
        scene.context.m_entityFactory = factory;
        game::RegisterGameComponents(scene.context);

        engine::MapLoader loader(scene.context);
        if (!loader.load(mapPath.string())) {
            stats.error = "failed to load map";
            return stats;
        }

        engine::PhysicsBodyCreationSystem bodyCreation(scene.context);
        engine::PhysicsEventSystem physicsEvents(scene.context);
        engine::PhysicsSyncSystem physicsSync(scene.context);
        engine::HierarchySystem hierarchy(scene.context);
        engine::LifetimeSystem lifetime(scene.context);
        game::VisibilitySystem visibility(scene.context);
        game::ai::EnemyDetectionSystem detection(scene.context);
        game::ai::EnemyStateTransitions state(scene.context);
        game::ai::EnemyMovementSystem movement(scene.context);
        game::ai::EnemyCombatSystem combat(scene.context);
        game::ai::EnemyStuckRecoverySystem stuck(scene.context);

        bodyCreation.update();

        struct StartPos {
            std::uint32_t id = 0;
            engine::Vector2f pos{};
        };
        std::vector<StartPos> starts;
        {
            auto view = scene.registry->view<game::EnemyComponent, engine::TransformComponent>();
            for (auto entity : view) {
                const auto& transform = view.get<engine::TransformComponent>(entity);
                starts.push_back({entt::to_integral(entity), transform.position});
            }
        }
        stats.enemyCount = static_cast<int>(starts.size());

        for (int frame = 0; frame < frames; ++frame) {
            bodyCreation.update();
            detection.update(dt);
            state.update(dt);
            movement.update(dt);
            combat.update(dt);
            stuck.update(dt);
            b2World_Step(scene.context.m_physicsWorld, dt, 8);
            physicsEvents.update();
            scene.dispatcher->update();
            physicsSync.update();
            visibility.update();
            hierarchy.update();
            lifetime.update(dt);
        }

        stats.stuckChecks = countSubstring(stats.stuckLog, "stuck check");
        stats.movedZero = countSubstring(stats.stuckLog, "moved 0.00");
        stats.teleports = countSubstring(stats.stuckLog, "teleport to");
        stats.impulses = countSubstring(stats.stuckLog, "impulse");
        stats.visibleImpulses = countSubstring(stats.stuckLog, "impulse instead");
        stats.recovered = countSubstring(stats.stuckLog, "recovered from stuck");
        stats.killed = countSubstring(stats.stuckLog, "PERMANENTLY STUCK");

        float minDisp = 1e9f;
        float maxDisp = 0.0f;
        auto view = scene.registry->view<game::EnemyComponent, engine::TransformComponent>();
        for (auto entity : view) {
            const auto& enemy = view.get<game::EnemyComponent>(entity);
            stats.maxStuck = std::max(stats.maxStuck, enemy.stuckCounter);
            const auto& transform = view.get<engine::TransformComponent>(entity);
            for (const auto& start : starts) {
                if (start.id != entt::to_integral(entity)) continue;
                const float dx = transform.position.x - start.pos.x;
                const float dy = transform.position.y - start.pos.y;
                const float disp = std::sqrt(dx * dx + dy * dy);
                minDisp = std::min(minDisp, disp);
                maxDisp = std::max(maxDisp, disp);
                if (disp < 10.0f && enemy.currentState != game::EnemyState::Dead) ++stats.barelyMoved;
                break;
            }
        }
        stats.minDisp = (minDisp > 1e8f) ? 0.0f : minDisp;
        stats.maxDisp = maxDisp;
        return stats;
    }

} // namespace baja::test
