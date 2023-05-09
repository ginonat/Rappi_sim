#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath> 

#include "../include/draw.h"
#include "../include/struct.h"



int main()
{
    // Create a window with a black background
    sf::RenderWindow window(sf::VideoMode(640, 480), "My Window", sf::Style::Titlebar | sf::Style::Close);
    window.clear(sf::Color::Black);


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

    // Create a vertex array for the streets
    sf::VertexArray streets(sf::Lines, 0);
    
    // Add the streets to the vertex array
    for (const auto& node : nodes) {
        for (const auto& neighbor : node.neighbors) {
            sf::Vertex vertex1(node.position, sf::Color::White);
            sf::Vertex vertex2(neighbor->position, sf::Color::White);
            streets.append(vertex1);
            streets.append(vertex2);
        }
    }
    
    // Set the outline thickness of the streets
    streets.setPrimitiveType(sf::Lines);
    
    // Create a red runner
    Runner runner_A(&nodes[35], sf::Vector2f(10, 10), sf::Color::Red);


    // sanity check
    // Print out the current node and its neighbors for each runner
    std::cout << "Current node position: " << runner_A.current_node->position.x << ", " << runner_A.current_node->position.y << std::endl;
    std::cout << "Neighbor nodes positions: ";
    for (const auto& n : runner_A.current_node->neighbors) {
        std::cout << "(" << n->position.x << ", " << n->position.y << ") ";
    }
    std::cout << std::endl;


    // Run the game loop
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

           // Check if the event is a key release
            if (event.type == sf::Event::KeyPressed) {
                // Check if the key pressed is a number key between 0 and the maximum neighbor index
                if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9) {
                    std::vector<Node*>::size_type neighborIndex = event.key.code - sf::Keyboard::Num0;
                    if (neighborIndex < runner_A.current_node->neighbors.size() && runner_A.running==false) {
                        runner_A.moveToNextNode(neighborIndex);
                        std::cout << "Runner A moved to node at position (" << runner_A.current_node->neighbors[neighborIndex]->position.x << ", " << runner_A.current_node->neighbors[neighborIndex]->position.y << ")" << std::endl;
                    }
                }
            }
        }

        // Clear the window
        window.clear(sf::Color::Black);

        // Interpolate runner position towards target node
        sf::Vector2f distance_to_target = runner_A.target_node->position - runner_A.box.getPosition();
        float distance = std::sqrt(distance_to_target.x * distance_to_target.x + distance_to_target.y * distance_to_target.y);
        if (distance > 0) {
            sf::Vector2f direction = distance_to_target / distance;
            sf::Vector2f velocity = direction * runner_A.movement_speed;
            sf::Vector2f new_position = runner_A.box.getPosition() + velocity;
            if (distance < runner_A.movement_speed) {
                new_position = runner_A.target_node->position;
                runner_A.current_node = runner_A.target_node;
                runner_A.running      = false;
            }
            runner_A.box.setPosition(new_position);
            std::cout << "My pos is: (" << new_position.x << ", " << new_position.y << ")" << std::endl;


        }

        // Draw the Nodes
        for (const auto& node : nodes_sprit) {
            window.draw(node);
            drawArrows(window, runner_A.current_node);
        }
        // Draw streets
        window.draw(streets);

        // Draw the runner
        window.draw(runner_A.box);

        // Display the window
        window.display();
    }

    return 0;
}

