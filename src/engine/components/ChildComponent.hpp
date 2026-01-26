#pragma once

#include <entt/entity/entity.hpp>
#include <vector>

namespace engine
{
    /**
     * @brief Component that maintains a list of children attached to this entity.
     *
     * This component creates a hierarchy relationship. It is useful for propagating
     * events (like destruction or transformation updates) from a parent entity
     * to its children (e.g., "destroy all children when I die" or "move inventory items with me").
     */
    struct ChildComponent
    {
        /**
         * @brief A collection of child entities owned by this entity.
         * * @note These entities should likely be destroyed or detached if the 
         * parent entity is destroyed.
         */
        std::vector<entt::entity> children;
    };
}