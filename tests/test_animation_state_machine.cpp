#include "engine/components/AnimationComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/systems/AnimationStateMachineSystem.hpp"

#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

#include <memory>
#include <string>

namespace {

    struct Harness {
        engine::EngineContext context;
        std::shared_ptr<entt::registry> registry = std::make_shared<entt::registry>();
        std::shared_ptr<engine::AnimationLibrary> library = std::make_shared<engine::AnimationLibrary>();
        std::unique_ptr<engine::AnimationStateMachineSystem> system;

        Harness() {
            context.m_registry = registry;
            context.m_animationLibrary = library;
            system = std::make_unique<engine::AnimationStateMachineSystem>(context);
        }

        void addClip(const std::string& id) {
            engine::AnimationClip clip;
            clip.name = id;
            library->registerClip(id, clip);
        }

        entt::entity spawn(const std::string& set, engine::AnimationState state,
                           engine::FacingDirection facing = engine::FacingDirection::Right, float scaleX = 1.0f) {
            auto e = registry->create();
            auto& animState = registry->emplace<engine::AnimationStateComponent>(e);
            animState.animationSetId = set;
            animState.state = state;
            animState.facing = facing;
            registry->emplace<engine::AnimationComponent>(e);
            auto& transform = registry->emplace<engine::TransformComponent>(e);
            transform.scale.x = scaleX;
            return e;
        }
    };

} // namespace

TEST_CASE("AnimationStateMachine picks the clip for a known set and state") {
    Harness h;
    h.addClip("player_idle");
    auto e = h.spawn("player", engine::AnimationState::Idle);
    h.system->update();
    CHECK(h.registry->get<engine::AnimationComponent>(e).currentClipId == "player_idle");
}

TEST_CASE("AnimationStateMachine prefers the _anim suffix when both clips exist") {
    Harness h;
    h.addClip("player_idle");
    h.addClip("player_idle_anim");
    auto e = h.spawn("player", engine::AnimationState::Idle);
    h.system->update();
    CHECK(h.registry->get<engine::AnimationComponent>(e).currentClipId == "player_idle_anim");
}

TEST_CASE("AnimationStateMachine strips underscore suffixes from the set name") {
    Harness h;
    h.addClip("player_idle");
    auto e = h.spawn("player_ak", engine::AnimationState::Idle);
    h.system->update();
    CHECK(h.registry->get<engine::AnimationComponent>(e).currentClipId == "player_idle");
}

TEST_CASE("AnimationStateMachine falls Aim and Heal back to Idle when those clips are missing") {
    Harness h;
    h.addClip("player_idle");
    auto aimer = h.spawn("player", engine::AnimationState::Aim);
    auto healer = h.spawn("player", engine::AnimationState::Heal);
    h.system->update();
    CHECK(h.registry->get<engine::AnimationComponent>(aimer).currentClipId == "player_idle");
    CHECK(h.registry->get<engine::AnimationComponent>(healer).currentClipId == "player_idle");
}

TEST_CASE("AnimationStateMachine returns the constructed id when nothing is in the library") {
    Harness h;
    auto e = h.spawn("player", engine::AnimationState::Walk);
    h.system->update();
    CHECK(h.registry->get<engine::AnimationComponent>(e).currentClipId == "player_walk_anim");
}

TEST_CASE("AnimationStateMachine flips scale when facing left and skips work after commit") {
    Harness h;
    h.addClip("player_idle");
    auto e = h.spawn("player", engine::AnimationState::Idle, engine::FacingDirection::Left, 2.0f);
    h.system->update();
    CHECK(h.registry->get<engine::TransformComponent>(e).scale.x == -2.0f);

    auto& anim = h.registry->get<engine::AnimationComponent>(e);
    anim.currentClipId = "marker";
    h.system->update();
    CHECK(anim.currentClipId == "marker");
}
