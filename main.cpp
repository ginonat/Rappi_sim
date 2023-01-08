#include <iostream>
#include <SFML/Graphics.hpp>

int main()
{
    // Create a window with a black background
    sf::RenderWindow window(sf::VideoMode(640, 480), "My Window", sf::Style::Titlebar | sf::Style::Close);
    window.clear(sf::Color::Black);

    // Create a red point
    sf::CircleShape point(5);
    point.setPosition(320, 240);
    point.setFillColor(sf::Color::Red);

    // Run the game loop
    while (window.isOpen())
    {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // Update the point's position based on the arrow keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            point.move(-.01, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            point.move(.01, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            point.move(0, -.01);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            point.move(0, .01);
        }

        // Clear the window
        window.clear(sf::Color::Black);

        // Draw the point
        window.draw(point);

        // Display the window
        window.display();
    }

    return 0;
}

