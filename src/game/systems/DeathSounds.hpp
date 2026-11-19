#pragma once

#include <array>
#include <string_view>

namespace game::death_sounds {

    // Sound ids played by DamageSystem when an entity dies. Every id here must
    // exist in the "sounds" section of assets/data/resources.json.
    inline constexpr std::string_view PLAYER = "death_player";
    inline constexpr std::array<std::string_view, 2> ENEMY = {"death_1", "death_2"};

} // namespace game::death_sounds
