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
    runner.setPosition(640, 480);
    runner.setFillColor(sf::Color::Red);
    // previous runner position
    sf::Vector2f prevRunnerPos;


    struct Node {
        sf::Vector2f position;
        std::vector<Node*> neighbors;
    };

    // Create nodes
    const int rows = 10;
    const int cols = 10;
    const float nodeSpacing_x = window.getSize().x / (cols-1);
    const float nodeSpacing_y = window.getSize().y / (rows-1);
    std::vector<Node> nodes(rows * cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Node& node = nodes[i * cols + j];
//            node.position = sf::Vector2f(j * nodeSpacing_x, i * nodeSpacing_y);
            // add some random to the nodes 
            node.position = sf::Vector2f(j * nodeSpacing_x + (rand()%100) * nodeSpacing_x/300, i * nodeSpacing_y+ (rand()%100) * nodeSpacing_y/300);

            // Connect to neighboring nodes
            if (i > 0) {
                node.neighbors.push_back(&nodes[(i - 1) * cols + j]);
            }
            if (j > 0) {
                node.neighbors.push_back(&nodes[i * cols + (j - 1)]);
            }
            if (i < rows - 1) {
                node.neighbors.push_back(&nodes[(i + 1) * cols + j]);
            }
            if (j < cols - 1) {
                node.neighbors.push_back(&nodes[i * cols + (j + 1)]);
            }
        }
    }

    // Create a circle shape for each node
    std::vector<sf::CircleShape> nodes_sprit;
    float nodeRadius = 2.0f;
    for (const auto& node : nodes ) {
        sf::CircleShape circle(nodeRadius);
        circle.setFillColor(sf::Color::White);
        circle.setPosition(node.position.x - nodeRadius, node.position.y - nodeRadius);
        nodes_sprit.push_back(circle);
    }

    // Create a vertex array for the lines
    sf::VertexArray lines(sf::Lines, 0);
    
    // Add the lines to the vertex array
    for (const auto& node : nodes) {
        for (const auto& neighbor : node.neighbors) {
            sf::Vertex vertex1(node.position, sf::Color::White);
            sf::Vertex vertex2(neighbor->position, sf::Color::White);
            lines.append(vertex1);
            lines.append(vertex2);
        }
    }
    
    // Set the outline thickness of the lines
    lines.setPrimitiveType(sf::Lines);



    // Set the movement speed to 0.01 pixels per frame
    float movementSpeed = 0.01f;

    // Run the game loop
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

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


//        // keep runer in the streets
//        in_street=false;
//        for(auto& street : streets){
//            if ((runner.getGlobalBounds().intersects(street.getGlobalBounds()))){
//                in_street=true;
//            }
//        }
//        if (!(in_street)){
//            runner.setPosition(prevRunnerPos);
//        }
//
//
        // Clear the window
        window.clear(sf::Color::Black);


        // Draw the Nodes
        for (const auto& node : nodes_sprit) {
            window.draw(node);
        }
        // Draw lines
//        window.draw(lines.data(), lines.size(), sf::Lines);
        window.draw(lines);

        // Draw the runner
        window.draw(runner);

        // Display the window
        window.display();
    }

    return 0;
}

