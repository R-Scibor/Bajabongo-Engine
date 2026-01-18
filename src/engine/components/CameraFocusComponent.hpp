#pragma once

namespace engine
{
    struct CameraFocusComponent
    {
        float viewHeight = 720.0f; // Height of the camera view in world units
        float targetViewHeight = 720.0f; // Target height for smooth zooming
        float smoothness = 0.1f; // Higher is faster (0.1 = 10% per frame)
        float minZoom = 200.0f;
        float maxZoom = 2000.0f;
        float zoomSpeed = 50.0f;
    };
}