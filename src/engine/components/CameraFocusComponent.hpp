#pragma once

namespace engine
{
    struct CameraFocusComponent
    {
        float zoom = 1.0f;
        float smoothness = 5.0f; // Higher is faster
    };
}