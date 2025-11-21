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

