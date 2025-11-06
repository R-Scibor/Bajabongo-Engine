#include "pch.h"
#include "engine.h"

Engine::Engine()
{
}

void Engine::run()
{
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        if (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        else
        {
            window.clear();
            window.draw(shape);
            window.display();
        }
    }
}
