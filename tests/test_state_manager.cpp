#include "engine/core/EngineContext.hpp"
#include "engine/core/IGameState.hpp"
#include "engine/core/StateManager.hpp"
#include "TestHelpers.hpp"

#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

    std::vector<std::string> g_log;

    class DummyState : public engine::IGameState {
    public:
        explicit DummyState(std::string name) : m_name(std::move(name)) {}

        void onEnter(engine::EngineContext&) override { g_log.push_back("enter:" + m_name); }
        void onExit(engine::EngineContext&) override { g_log.push_back("exit:" + m_name); }
        void handleEvent(engine::EngineContext&, const sf::Event&) override {}
        void update(engine::EngineContext&, float) override { g_log.push_back("update:" + m_name); }
        void render(engine::EngineContext&) override { g_log.push_back("render:" + m_name); }

    private:
        std::string m_name;
    };

    struct MenuDummy : DummyState {
        MenuDummy() : DummyState("Menu") {}
    };
    struct PlayDummy : DummyState {
        PlayDummy() : DummyState("Play") {}
    };
    struct PauseDummy : DummyState {
        PauseDummy() : DummyState("Pause") {}
    };

    struct Harness {
        engine::EngineContext context;
        std::shared_ptr<baja::test::NullLoggerManager> logs = std::make_shared<baja::test::NullLoggerManager>();
        std::shared_ptr<entt::dispatcher> dispatcher = std::make_shared<entt::dispatcher>();
        std::unique_ptr<engine::StateManager> manager;

        Harness() {
            context.m_logManager = logs;
            context.m_dispatcher = dispatcher;
            manager = std::make_unique<engine::StateManager>(context);
            manager->registerState<MenuDummy>("Menu");
            manager->registerState<PlayDummy>("Play");
            manager->registerState<PauseDummy>("Pause");
        }
    };

} // namespace

TEST_CASE("StateManager defers push/pop/swap until processTransitions") {
    Harness h;
    g_log.clear();

    CHECK(h.manager->isEmpty());
    h.manager->requestPush("Menu");
    CHECK(h.manager->isEmpty());
    CHECK(g_log.empty());

    h.manager->processTransitions();
    CHECK_FALSE(h.manager->isEmpty());
    CHECK(g_log == std::vector<std::string>{"enter:Menu"});

    h.manager->requestPush("Play");
    h.manager->processTransitions();
    CHECK(g_log == (std::vector<std::string>{"enter:Menu", "enter:Play"}));

    // Only the top state updates; both still render bottom to top.
    g_log.clear();
    h.manager->update(0.016f);
    h.manager->render();
    CHECK(g_log == (std::vector<std::string>{"update:Play", "render:Menu", "render:Play"}));

    g_log.clear();
    h.manager->requestPop();
    h.manager->processTransitions();
    CHECK(g_log == std::vector<std::string>{"exit:Play"});
    CHECK_FALSE(h.manager->isEmpty());

    g_log.clear();
    h.manager->requestSwap("Pause");
    h.manager->processTransitions();
    CHECK(g_log == (std::vector<std::string>{"exit:Menu", "enter:Pause"}));

    g_log.clear();
    h.manager->requestClear();
    h.manager->processTransitions();
    CHECK(g_log == std::vector<std::string>{"exit:Pause"});
    CHECK(h.manager->isEmpty());
}

TEST_CASE("StateManager ignores an unknown state name") {
    Harness h;
    g_log.clear();
    h.manager->requestPush("DoesNotExist");
    h.manager->processTransitions();
    CHECK(h.manager->isEmpty());
    CHECK(g_log.empty());
}
