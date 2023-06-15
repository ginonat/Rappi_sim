#include <iostream>
#include <limits>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath> 

#include "../include/draw.h"
#include "../include/struct.h"



int main()
{
    // Create a window with a black background
    sf::RenderWindow window(sf::VideoMode(640, 480), "My Window", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.clear(sf::Color::Black);

    // create a view object
    sf::View view(window.getDefaultView());
    window.setView(view);

    // zoom and view movement parameters
    float zoomFactor = std::pow(1.1f, 1);
    float delta;
    float zoom=1;


    // edit mode set so false 
    bool edit_mode = false;
    sf::CircleShape nodeCircle(5.0f);
    Node* closestNode = nullptr;
    float shopRadius=5;
    sf::CircleShape shopCircle(shopRadius);

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
    sf::CircleShape circle(nodeRadius);
    for (const auto& node : nodes ) {
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
    std::vector<Runner> runners;


    // Run the game loop
    while (window.isOpen()) {
        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
            } else if (event.type == sf::Event::MouseWheelMoved) {
                sf::Vector2f mousePosition(sf::Mouse::getPosition(window));
                sf::Vector2f center(view.getCenter());
                delta = event.mouseWheel.delta;
                zoomFactor = std::pow(1.1f, delta);
                zoom = zoom * zoomFactor;
                view.setSize(window.getDefaultView().getSize() / zoom);
                view.setCenter((mousePosition));
                std::cout << "setCenter x: " << window.getDefaultView().getSize().x << "zoom" << zoomFactor << window.getDefaultView().getSize().x / zoomFactor << std::endl;
                window.setView(view);
            }

           // check for edit mode in every event keyPressed
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::E) {
                    edit_mode = !edit_mode;
                    if (edit_mode) {
                        window.setTitle("Paused: Edit mode on");
                    } else {
                        window.setTitle("Unpaused: Edit mode off");
                    }
                }
            }
            if (edit_mode) { 
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    float closestDist = std::numeric_limits<float>::max();
                    for (auto& node : nodes) {
                        float dist = std::sqrt(std::pow(node.position.x - mousePos.x, 2) + std::pow(node.position.y - mousePos.y, 2));
                        if (dist < closestDist) {
                            closestDist = dist;
                            closestNode = &node;
                        }
                    }
                    std::cout << "Selected node: position=(" << closestNode->position.x << "," << closestNode->position.y << ")"  << std::endl;
                    
                    // Highlight the selected node by turning red
                    nodeCircle.setFillColor(sf::Color::Red);
                    nodeCircle.setPosition(closestNode->position.x - 5.0f, closestNode->position.y - 5.0f);
                    }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) and closestNode!=nullptr) {
                    closestNode->has_shop = true;
                    std::cout << "test"  << std::endl;
                    shopCircle.setFillColor(sf::Color::White);
                    shopCircle.setPosition(closestNode->position.x - shopRadius, closestNode->position.y - shopRadius);
                    nodes_sprit.push_back(shopCircle);
                }
            } else { 
                if (event.type == sf::Event::KeyPressed) {
                    // Add new runner
                    if (event.key.code >= sf::Keyboard::Add) {
                        Node* start_node = &nodes[rand() % nodes.size()];
                        runners.emplace_back(start_node, sf::Vector2f(10, 10), sf::Color::Blue, 0.03f);
                    }
                }
            }
        }
        if (!edit_mode) { 
            // Check if the event is a keyPressed
            for (auto& runner : runners) {
                // Check if the runner has reached its target node
                if (runner.running && runner.box.getPosition() == runner.target_node->position) {
                    runner.current_node = runner.target_node;
                    runner.running = false;
                }
                // If the runner is not currently running, select a new target node
                if (!runner.running) {
                    runner.moveToNextNode();
                }
            }
        }

        // Clear the window
        window.clear(sf::Color::Black);

        // Interpolate runner position towards target node

        for (auto& runner : runners) {
            sf::Vector2f distance_to_target = runner.target_node->position - runner.box.getPosition();
            float distance = std::sqrt(distance_to_target.x * distance_to_target.x + distance_to_target.y * distance_to_target.y);
            if (distance > 0) {
                sf::Vector2f direction = distance_to_target / distance;
                sf::Vector2f velocity = direction * runner.movement_speed;
                sf::Vector2f new_position = runner.box.getPosition() + velocity;
                if (distance < runner.movement_speed) {
                    new_position = runner.target_node->position;
                    runner.current_node = runner.target_node;
                    runner.running      = false;
                }
                runner.box.setPosition(new_position);
                //std::cout << "My pos is: (" << new_position.x << ", " << new_position.y << ")" << std::endl;
            }

        }

        // Draw the Nodes
        for (const auto& node : nodes_sprit) {
            window.draw(node);
        }

        // Draw streets
        window.draw(streets);

        // Draw the runners
        for (auto& runner : runners) {
            // Update runner position if it is running
            if (runner.running) {
                // ...
            }

            // Draw the runner
            window.draw(runner.box);
        }
        if (edit_mode and (nodeCircle.getPosition() != sf::Vector2f(0,0)) ){
            window.draw(nodeCircle);
        }

        // Display the window
        window.display();
    }

    return 0;
}

