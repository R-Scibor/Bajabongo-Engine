#include "engine/core/Application.h"
// Nie potrzebujemy już SFMLWindow.h
#include "engine/rendering/SFMLRenderer.h" // <-- Ta klasa robi teraz wszystko
#include <iostream> // Dla obsługi wyjątków

int main() {
    try {
        // 1. Instancjonujemy naszą jedną, konkretną klasę implementacyjną
        SFMLRenderer renderer; // <-- ZMIENIONE

        // 2. Tworzymy okno
        renderer.create("Game Window", 1280, 720); // <-- ZMIENIONE

        // 3. Wstrzykujemy 'renderer' jako *oba* wymagane interfejsy
        // Application będzie widzieć 'renderer' raz jako IWindow, a raz jako IRenderer
        Application app(renderer, renderer); // <-- ZMIENIONE

        // 4. Uruchamiamy aplikację
        app.run();

    }
    catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}