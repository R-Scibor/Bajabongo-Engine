#pragma once

namespace game
{
    struct PlayerComponent
    {
        float moveSpeed = 5.0f; // Speed in physics units per second
        float rotSpeed = 10.0f; // Rotation speed (if using torque/angular velocity) - though we might set rotation directly
        
        // Healing
        int medkits = 2;        // Resource count
        int maxMedkits = 5;     // Max carry limit
        bool isHealing = false; // State flag
        float healTimer = 0.0f; // Countdown for the 3s heal channel
    };
}