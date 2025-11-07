#include "engine/core/Application.h"
#include "engine/rendering/SFMLRenderer.h" 
#include <iostream> 

int main() {
    try {
        SFMLRenderer renderer; 

        renderer.create("Game Window", 1280, 720); 

        Application app(renderer, renderer); 

        app.run();

    }
    catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}