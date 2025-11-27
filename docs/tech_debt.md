Decouple state machine from SFML by introducing engine::InputEvent and translating from backend events in IInputManager.

Write event-based InputManager.

Write events for collision system in Box2d.

Animation System: [
        Complex animation state machines (idle/run/shoot/reload layers, blend trees).

        Directional sprite selection (N/S/E/W facing) beyond maybe a simple hack if needed.

        Networking/determinism concerns around animation (they can be purely cosmetic for now).

        Any coupling to gameplay logic (e.g., “fire on frame 3” events) — that belongs in a more advanced “Gameplay & Combat” phase.
    ]

Physics & Collision (Top-Down Shooter Specifics): [
"The Feet Problem" (Hitbox Duality):
- OBECNY STAN: Używamy jednego kształtu (Box) wycentrowanego na pozycji entity.
- PROBLEM: W grze top-down (2.5D) potrzebujemy dwóch różnych zachowań kolizji:
1. Movement Collider (Stopy): Mały, solidny kształt u dołu sprite'a. Pozwala głowie "wchodzić" na ściany (efekt perspektywy) i przechodzić przez wąskie drzwi.
2. Hurtbox (Ciało): Duży sensor pokrywający cały sprite. Nie blokuje ruchu, ale wykrywa trafienia pocisków.

- ROZWIĄZANIE DOCELOWE (Faza 6/7):
    - Wdrożenie "Compound Bodies" w Box2D (jedno ciało, wiele kształtów).
    - Ustawienie filtrów kolizji (Bitmasking):
        - Kategoria FEET koliduje z ENVIRONMENT.
        - Kategoria HURTBOX (sensor) koliduje z PROJECTILE.
        
- TYMCZASOWE ROZWIĄZANIE (Faza 5A - Rendering Polish):
    - Ustawienie Origin sprite'a na "stopy" (dół-środek), aby grafika renderowała się "nad" punktem fizycznym.
    - Dzięki temu transform.position będzie wskazywał na stopy postaci, co ułatwi sortowanie Y.


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

    Rozwiązanie docelowe: System Zarządzania Scenami z obsługą tagów DontDestroyOnLoad lub pełna serializacja stanu gracza do pliku przed przeładowaniem sceny.