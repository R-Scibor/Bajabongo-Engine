#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <memory>
#include <string>

namespace baja::test {

    class NullLogger final : public engine::ILogger {
    public:
        void log(spdlog::level::level_enum, std::string_view) override {}
    };

    class NullLoggerManager final : public engine::ILoggerManager {
    public:
        NullLoggerManager() : m_logger(std::make_shared<NullLogger>()) {}

        std::shared_ptr<engine::ILogger> getLogger(const std::string&) override { return m_logger; }

    private:
        std::shared_ptr<engine::ILogger> m_logger;
    };

    /// EngineContext plus a gravity-zero Box2D world. No window.
    struct HeadlessScene {
        std::shared_ptr<entt::registry> registry = std::make_shared<entt::registry>();
        std::shared_ptr<entt::dispatcher> dispatcher = std::make_shared<entt::dispatcher>();
        std::shared_ptr<NullLoggerManager> logManager = std::make_shared<NullLoggerManager>();
        engine::EngineContext context;
        b2WorldId worldId{};

        HeadlessScene() {
            b2WorldDef worldDef = b2DefaultWorldDef();
            worldDef.gravity = {0.0f, 0.0f};
            worldId = b2CreateWorld(&worldDef);

            context.m_registry = registry;
            context.m_dispatcher = dispatcher;
            context.m_logManager = logManager;
            context.m_physicsWorld = worldId;
        }

        HeadlessScene(const HeadlessScene&) = delete;
        HeadlessScene& operator=(const HeadlessScene&) = delete;

        ~HeadlessScene() {
            if (b2World_IsValid(worldId)) {
                b2DestroyWorld(worldId);
            }
        }

        b2BodyId addStaticBox(float x, float y, float halfW, float halfH, unsigned int categoryBits) {
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_staticBody;
            bodyDef.position = {x, y};
            b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

            b2Polygon box = b2MakeBox(halfW, halfH);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.filter.categoryBits = categoryBits;
            shapeDef.filter.maskBits = 0xFFFFFFFFu;
            b2CreatePolygonShape(bodyId, &shapeDef, &box);
            return bodyId;
        }
    };

} // namespace baja::test
