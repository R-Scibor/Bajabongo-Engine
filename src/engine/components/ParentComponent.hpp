#pragma once

#include "engine/core/math/MathAliases.hpp"
#include <entt/entity/entity.hpp>

namespace engine
{
    /**
     * @brief Component that establishes a parent-child relationship for transforms.
     *
     * Attaches this entity to a parent entity. The entity's final world transform 
     * is calculated by combining the parent's world transform with these local offsets.
     * If the parent moves or rotates, this entity follows.
     */
    struct ParentComponent
    {
        /// @brief The entity ID of the parent. Set to entt::null if detached.
        entt::entity parentId = entt::null;

        /// @brief Position offset relative to the parent's origin (local space).
        Vector2f localPosition = { 0.0f, 0.0f };

        /// @brief Rotation offset relative to the parent's rotation (in radians/degrees depending on system).
        float localRotation = 0.0f;
    };
}