#pragma once

namespace engine
{
    /**
     * @brief Component marking an entity as a target for the camera to follow or focus on.
     * * Contains settings for camera zoom levels (view height) and smoothing parameters
     * for cinematic or gameplay camera movements.
     */
    struct CameraFocusComponent
    {
        /**
         * @brief Current height of the camera view in world units.
         * * Determines the zoom level. Smaller values mean "zoomed in", 
         * larger values mean "zoomed out".
         */
        float viewHeight = 720.0f; 

        /// @brief The target height the camera is trying to reach smoothly.
        float targetViewHeight = 720.0f; 

        /**
         * @brief Interpolation factor for smooth zooming.
         * * Value ideally between 0.0 and 1.0. 
         * Example: 0.1 means the camera covers 10% of the difference per frame (or tick).
         */
        float smoothness = 0.1f; 

        /// @brief The maximum zoom-in level (minimum view height).
        float minZoom = 200.0f;

        /// @brief The maximum zoom-out level (maximum view height).
        float maxZoom = 2000.0f;

        /// @brief The speed at which the zoom changes when triggered by input.
        float zoomSpeed = 50.0f;
    };
}