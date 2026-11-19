Decouple state machine from SFML by introducing engine::InputEvent and translating from backend events in IInputManager.

Write event-based InputManager.

Write events for collision system in Box2d.

Animation System: [
        Complex animation state machines (idle/run/shoot/reload layers, blend trees).

        Directional sprite selection (N/S/E/W facing) beyond maybe a simple hack if needed.

        Networking/determinism concerns around animation (they can be purely cosmetic for now).

        Any coupling to gameplay logic (e.g., “fire on frame 3” events) — that belongs in a more advanced “Gameplay & Combat” phase.

        **Single-Frame Clips Requirement**: The `AnimationSystem` strictly requires an `AnimationClip` to exist for any state transition. If an entity transitions to "Idle" but only has a static sprite and no corresponding 1-frame animation clip, the sprite will not update (it will freeze on the last frame of the previous animation). We currently generate these clips automatically in `AssetBaker.py` for all static sprites, but a more robust engine-level fallback (e.g. play static sprite if clip missing) would be cleaner.

        **Hardcoded Naming Conventions**: `AnimationStateMachineSystem` relies on strict string concatenation (`id + "_idle"`, `id + "_walk_anim"`). This is fragile. Ideally, we should have a data-driven mapping or a more flexible Animation Graph system.
    ]

Physics & Collision (Top-Down Shooter Specifics): [
"The Feet Problem" (Hitbox Duality):
- **SOLVED** (Jan 2026): Implemented multi-fixture bodies and collision categories.
    - **Feet**: Physical collider (Category: Player/Enemy) that interacts with Environment (Walls, LowObstacles).
    - **Hurtbox**: Sensor collider (Category: Hurtbox) that interacts with Projectiles.
    - **LowObstacles**: New category for fences/tables that block movement but allow shooting.
        
- NEXT STEPS:
    - Fine-tune shape sizes in `archetypes.json`.
    - Implement visual debug drawing for specific fixtures if needed.
]

Add pushable objects, using density in PhysicsBodyComponent

Persistence & Serialization (Save System): [
Currently, entities spawned via the Debug Spawner are lost when the game closes.

Requirement:
- Implement a Save/Load system that serializes the relevant parts of the entt::registry to a file (JSON or Binary).
- Must support serializing core components: Transform, Renderable, Physics properties (to reconstruct bodies), Health, etc.
- Differentiation between "Static World" (loaded from LDtk) and "Dynamic State" (save game data).

Implementation Strategy:
- Evaluate 'cereal' library for automatic serialization vs manual JSON mapping.
- Needs a "Post-Load" system to recreate Box2D bodies from the loaded data, as physics handles (b2BodyId) cannot be saved directly.



ECS & Hierarchy: [
    Optimization:
    - HierarchySystem currently iterates all ParentComponents. For very deep hierarchies, we might need topological sorting or breadth-first traversal to ensure parents update before children in a single frame (currently if child updates before parent, it lags one frame).
    - Dirty Flags: Only update children if parent moved.

    Features:
    - Detaching: Currently we only support destroying children. We need a way to "Drop" an item (detach from parent, keep world transform).
]
]

Sekcja: Rendering & Optimization

Issue: Brak Frustum Culling i partycjonowania przestrzennego

    Status: Odroczone (Low Priority dla małych map)

    Opis: Obecny RenderSystem iteruje przez wszystkie encje posiadające komponenty Transform i Renderable w każdej klatce. Przy małych mapach (kilkaset obiektów) jest to akceptowalne. Jednak przy większych światach (Phase 6+) spowoduje to drastyczny spadek FPS, ponieważ GPU będzie przetwarzać obiekty znajdujące się poza ekranem.   

Rozwiązanie docelowe: Implementacja struktury Quadtree lub Spatial Hash Grid oraz sprawdzanie viewRect.intersects(entityRect) przed wysłaniem obiektu do renderowania.

Trigger do naprawy: Spadek wydajności poniżej 60 FPS przy powiększeniu mapy.

Sekcja: Architecture & Lifecycle

Issue: Tymczasowe czyszczenie świata (Brute-force Scene Clear)

    Status: Tymczasowe rozwiązanie (Temporary Hack)

    Opis: Zamiast pełnej serializacji lub selektywnego ładowania scen, stosujemy podejście "Nuke the World". Przy wyjściu ze stanu (GameplayState::onExit) wywoływane jest registry.clear(), co usuwa wszystkie encje, w tym potencjalne systemy globalne (np. odtwarzacz muzyki, statystyki sesji).

    Ograniczenia:

        Uniemożliwia przenoszenie stanu gracza (HP, Ekwipunek) między poziomami bez zewnętrznego "Global Context".

        Wymaga ponownego inicjowania wszystkich zasobów przy każdym wejściu do gry.


Section: Assets & Loading

Issue: Hardcoded Map Loader
    Status: Temporary Hack (Phase 5C - Map Milestone)
    Description: The current `MapLoader` class (`src/engine/assets/MapLoader.cpp`) is a temporary solution designed to load a specific `map.json` (Tiled export) for the milestone. It contains hardcoded scaling factors (4.0f) and relies on specific layer names ("Collision", "Image Layer 1") and resource IDs ("map_sprite").
    Limitations:
        - Does not support generic Tiled map loading (multiple layers, tilesets, object types).
        - Scaling is hardcoded in C++ instead of being data-driven or handled by the camera/viewport.
        - Does not integrate with a proper Level/Scene management system.
    Target Solution: Implement a robust Level Loader that can parse generic Tiled JSON exports, handle multiple layers, tilesets, and object properties dynamically. It should integrate with the resource manager to load required textures on demand.
    Rozwiązanie docelowe: System Zarządzania Scenami z obsługą tagów DontDestroyOnLoad lub pełna serializacja stanu gracza do pliku przed przeładowaniem sceny.