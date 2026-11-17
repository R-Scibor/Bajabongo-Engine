#pragma once

// The component lives in the engine (MapLoader / RenderSystem need it).
// This header keeps existing game includes and the vcxproj entry working.
#include "engine/components/VisibleToPlayerComponent.hpp"

namespace game
{
    using VisibleToPlayerComponent = engine::VisibleToPlayerComponent;
}
