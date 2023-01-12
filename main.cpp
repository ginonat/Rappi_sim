#include <iostream>
#include <SFML/Graphics.hpp>

int main()
{
    // Create a window with a black background
    sf::RenderWindow window(sf::VideoMode(640, 480), "My Window", sf::Style::Titlebar | sf::Style::Close);
    window.clear(sf::Color::Black);

    // Create a green box used for sanity checks
    sf::RectangleShape SanityCheck(sf::Vector2f(10,10));
    SanityCheck.setPosition(0, 0);
    SanityCheck.setFillColor(sf::Color::Green);

    // Create a red runner
    sf::RectangleShape runner(sf::Vector2f(10,10));
    runner.setPosition(320, 240);
    runner.setFillColor(sf::Color::Red);
    // previous runner position
    sf::Vector2f prevRunnerPos;

    bool in_street=false;

    // Create a streets  
    std::vector<sf::RectangleShape> streets;
    int rows = 10;
    int cols = 10;
//    float streetSize_x = runner.getSize().x;
//    float streetSize_y = runner.getSize().y;
    
    for (int i = 0; i < rows; i++) {
        sf::RectangleShape street(sf::Vector2f(window.getSize().x, 1));
        street.setPosition(0, i * window.getSize().y / rows);
        street.setFillColor(sf::Color::White);
        streets.push_back(street);
    }
    for (int j = 0; j < cols; j++) {
        sf::RectangleShape street(sf::Vector2f(1, window.getSize().y));
        street.setPosition(j * window.getSize().x / cols, 0);
        street.setFillColor(sf::Color::White);
        streets.push_back(street);
    }
// matrix of points
//    for (int i = 0; i < rows; i++) {
//        for (int j = 0; j < cols; j++) {
//            sf::RectangleShape street(sf::Vector2f(1, 5));
//            street.setPosition(j * streetSize_y, i * streetSize_x);
//            street.setFillColor(sf::Color::White);
//            streets.push_back(street);
//        }
//    }

    // Set the movement speed to 0.01 pixels per frame
    float movementSpeed =0.01f;

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

        // store previous runner position
        prevRunnerPos=runner.getPosition();

        // Check if the shift key is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))
        {
            // Increase the movement speed if the shift key is pressed
            movementSpeed = 0.03f;
        }
    else
    {
            // Reset the movement speed if the shift key is not pressed
            movementSpeed = 0.01f;
        }
        // Update the runner's position based on the arrow keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            runner.move(-movementSpeed, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            runner.move(movementSpeed, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            runner.move(0, -movementSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            runner.move(0, movementSpeed);
        }

        // Stop the runner from leaving the window

        if (runner.getPosition().x < 0)
            runner.setPosition(0, runner.getPosition().y);
        if (runner.getPosition().y < 0)
            runner.setPosition(runner.getPosition().x, 0);
        if (runner.getPosition().x + runner.getSize().x > window.getSize().x)
            runner.setPosition(window.getSize().x - runner.getSize().x, runner.getPosition().y);
        if (runner.getPosition().y + runner.getSize().y > window.getSize().y)
            runner.setPosition(runner.getPosition().x, window.getSize().y - runner.getSize().y);


        // keep runer in the streets
        in_street=false;
        for(auto& street : streets){
            if ((runner.getGlobalBounds().intersects(street.getGlobalBounds()))){
                in_street=true;
            }
        }
        if (!(in_street)){
            runner.setPosition(prevRunnerPos);
        }


        // Clear the window
        window.clear(sf::Color::Black);

        //sanity check
        in_street=false;
        for(auto& street : streets){
            if ((runner.getGlobalBounds().intersects(street.getGlobalBounds()))){
                in_street=true;
            }
        }
        if (in_street){
            window.draw(SanityCheck);
        }

        // Draw the street
        for (auto& street : streets)
            window.draw(street);

        // Draw the runner
        window.draw(runner);

        // Display the window
        window.display();
    }

    return 0;
}

