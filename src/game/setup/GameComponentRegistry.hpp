#pragma once

namespace engine {
    struct EngineContext;
    class EditorSystem;
}

namespace game {
    // Funkcja, która rejestruje wszystkie nasze game-specific loadery
    void RegisterGameComponents(engine::EngineContext& context);

    // Game-specific ImGui inspectors (Health / Weapon / Player). Engine editors
    // stay agnostic of these types.
    void RegisterGameEditorInspectors(engine::EditorSystem& editor);
}
