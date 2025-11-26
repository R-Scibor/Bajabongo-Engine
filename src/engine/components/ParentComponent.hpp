#pragma once

#include "engine/core/math/MathAliases.hpp"
#include <entt/entity/entity.hpp>

namespace engine
{
    /**
     * @brief Component that attaches this entity to a parent entity.
     *
     * The entity's world transform will be calculated based on the parent's
     * world transform and these local offsets.
     */
    struct ParentComponent
    {
        entt::entity parentId = entt::null;
        Vector2f localPosition = { 0.0f, 0.0f };
        float localRotation = 0.0f;
    };
}