#include <iostream>
#include <limits>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath> 
#include <fstream>

#include "../include/draw.h"
#include "../include/struct.h"
#include "../include/buildCity.h"

std::vector<Node> loadNodes(const std::string& filename) {
    std::vector<Node> nodes;
    std::ifstream file(filename);
    if (file.is_open()) {
        float x, y;
        bool has_shop;
        while (file >> x >> y >> has_shop) {
            Node node;
            node.position = sf::Vector2f(x, y);
            node.has_shop = has_shop;
            nodes.push_back(node);
        }
        file.close();
        std::cout << "Nodes loaded from " << filename << std::endl;
    } else {
        std::cout << "Unable to open file " << filename << std::endl;
    }
    return nodes;
}

void saveNodes(const std::vector<Node>& nodes, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& node : nodes) {
            file << node.position.x << " " << node.position.y << " " << node.has_shop << "\n";
        }
        file.close();
        std::cout << "Nodes saved to " << filename << std::endl;
    } else {
        std::cout << "Unable to open file " << filename << std::endl;
    }
}

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
    sf::CircleShape nodeSelect(5.0f);
    Node* closestNode = nullptr;
    float shopRadius=5;
    sf::CircleShape shopCircle(shopRadius);
    shopCircle.setFillColor(sf::Color::Yellow);

    // Create nodes
    const int rows = 20;
    const int cols = 20;
    std::vector<Node> nodes = loadNodes("maps/city.map"); // Load nodes from file
    if (nodes.empty()) {
        nodes = createNodes(rows, cols, window);
    }

    // Create a circle shape for the nodes
    float nodeRadius = 2.0f;
    sf::CircleShape nodeCircle(nodeRadius);
    nodeCircle.setFillColor(sf::Color::White);

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
    
    // Create a vector to store the red runners
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
                } else if (edit_mode && event.key.code == sf::Keyboard::L) {
                    nodes = loadNodes("city.map"); // Load nodes from file
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
                    nodeSelect.setFillColor(sf::Color::Red);
                    nodeSelect.setPosition(closestNode->position.x - 5.0f, closestNode->position.y - 5.0f);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) and closestNode!=nullptr) {
                    closestNode->has_shop = true;
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

        drawCity(window, nodes, nodeCircle, streets, shopCircle);
        drawRunners(window, runners);

        if (edit_mode and (nodeSelect.getPosition() != sf::Vector2f(0,0)) ){
            window.draw(nodeSelect);
        }
        if (edit_mode and (sf::Keyboard::isKeyPressed(sf::Keyboard::S))) {
            saveNodes(nodes, "new_city.map");
        }

        // Display the window
        window.display();
    }

    return 0;
}

