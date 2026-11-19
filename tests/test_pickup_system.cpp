#include "HeadlessScene.hpp"
#include "game/components/MedkitPickupComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/systems/PickupSystem.hpp"
#include "engine/events/PhysicsEvents.hpp"

#include <catch2/catch_test_macros.hpp>

#include <memory>

TEST_CASE("PickupSystem disconnects from the dispatcher when destroyed") {
    baja::test::HeadlessScene scene;
    {
        game::PickupSystem system(scene.context);
        CHECK_FALSE(scene.dispatcher->sink<engine::PhysicsSensorBeginEvent>().empty());
    }
    // GameplayState owns a PickupSystem. Leaving it (hideout -> hospital) must not
    // leave a dangling listener that the next sensor event would call.
    REQUIRE(scene.dispatcher->sink<engine::PhysicsSensorBeginEvent>().empty());
}

TEST_CASE("PickupSystem from a previous gameplay state is not called after a state swap") {
    baja::test::HeadlessScene scene;
    auto previous = std::make_unique<game::PickupSystem>(scene.context);
    previous.reset();
    REQUIRE(scene.dispatcher->sink<engine::PhysicsSensorBeginEvent>().empty());

    game::PickupSystem current(scene.context);

    auto player = scene.registry->create();
    auto& playerComp = scene.registry->emplace<game::PlayerComponent>(player);
    playerComp.medkits = 0;
    playerComp.maxMedkits = 5;
    auto pickup = scene.registry->create();
    scene.registry->emplace<game::MedkitPickupComponent>(pickup).medkitsGiven = 1;

    scene.dispatcher->enqueue<engine::PhysicsSensorBeginEvent>(pickup, player, false);
    scene.dispatcher->update();

    CHECK(playerComp.medkits == 1);
    CHECK_FALSE(scene.registry->valid(pickup));
}
