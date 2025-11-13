#pragma once

class b2Body;

namespace engine
{
    struct PhysicsBodyComponent
    {
        b2Body* body = nullptr;
    };
}