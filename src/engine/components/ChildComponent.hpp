#pragma once

#include <entt/entity/entity.hpp>
#include <vector>

namespace engine
{
    /**
     * @brief Component that maintains a list of children attached to this entity.
     *
     * Useful for gameplay logic like "destroy all children when I die" or
     * "iterate over my inventory items".
     */
    struct ChildComponent
    {
        std::vector<entt::entity> children;
    };
}