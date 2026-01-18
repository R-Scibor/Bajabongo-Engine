#pragma once
#include <string>
#include <entt/entity/entity.hpp>

namespace game {

    struct MapMarkerComponent {
        std::string mapConfigPath;
        std::string mapDisplayName;
        entt::entity textEntity = entt::null;
        float originalY = 0.0f;
        bool isHovered = false;
    };

} // namespace game
