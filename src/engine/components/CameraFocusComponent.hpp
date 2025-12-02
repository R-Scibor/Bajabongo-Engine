#pragma once

namespace engine
{
    struct CameraFocusComponent
    {
        float viewHeight = 720.0f; // Height of the camera view in world units
        float smoothness = 0.1f; // Higher is faster (0.1 = 10% per frame)
    };
}