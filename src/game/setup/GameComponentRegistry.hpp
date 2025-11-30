#pragma once

namespace engine { struct EngineContext; }

namespace game {
    // Funkcja, która rejestruje wszystkie nasze game-specific loadery
    void RegisterGameComponents(engine::EngineContext& context);
}