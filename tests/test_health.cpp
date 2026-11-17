#include "game/components/HealthComponent.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("applyDamage subtracts from current HP and detects defeat at zero or below") {
    game::HealthComponent health;
    health.currentHp = 100.0f;
    health.maxHp = 100.0f;

    game::applyDamage(health, 40.0f);
    CHECK(health.currentHp == 60.0f);
    CHECK_FALSE(game::isDefeated(health));

    game::applyDamage(health, 60.0f);
    CHECK(health.currentHp == 0.0f);
    CHECK(game::isDefeated(health));

    game::applyDamage(health, 5.0f);
    CHECK(health.currentHp == -5.0f);
    CHECK(game::isDefeated(health));
    CHECK(health.maxHp == 100.0f);
}
