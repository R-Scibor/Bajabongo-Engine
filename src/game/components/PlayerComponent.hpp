#pragma once

namespace game
{
    struct PlayerComponent
    {
        float moveSpeed = 5.0f; // Speed in physics units per second
        float rotSpeed = 10.0f; // Rotation speed (if using torque/angular velocity) - though we might set rotation directly
    };
}