#include <iostream>
#include <limits>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath> 
#include <fstream>

#include "../include/draw.h"
#include "../include/struct.h"
#include "../include/buildCity.h"


int main()
{
    // Create a window with a black background
    sf::RenderWindow window(sf::VideoMode(640, 480), "My Window", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.clear(sf::Color::Black);

    // create a view object
    sf::View view(window.getDefaultView());
    window.setView(view);

    // fixed-to-screen view for the sidebar HUD, unaffected by zoom/pan
    sf::View uiView(window.getDefaultView());
    const float sidebarWidth = 200.f;

    sf::Font font;
    bool fontLoaded = font.loadFromFile("assets/arial.ttf");
    if (!fontLoaded) {
        std::cerr << "Warning: failed to load assets/arial.ttf, sidebar text will not render" << std::endl;
    }

    // zoom and view movement parameters
    float zoomFactor = std::pow(1.1f, 1);
    float delta;
    float zoom=1;

    // middle-mouse-button drag panning
    bool panning = false;
    sf::Vector2f panAnchorWorld;


    // edit mode set so false
    bool edit_mode = false;
    const float nodePickRadius = 20.f; // pixels, screen-space snap radius for node picking
    sf::CircleShape nodeSelect(7.0f);
    nodeSelect.setFillColor(sf::Color::Transparent);
    nodeSelect.setOutlineColor(sf::Color::Red);
    nodeSelect.setOutlineThickness(2.f);
    sf::CircleShape nodeHover(7.0f);
    nodeHover.setFillColor(sf::Color::Transparent);
    nodeHover.setOutlineColor(sf::Color(0, 200, 255, 200));
    nodeHover.setOutlineThickness(1.5f);
    Node* closestNode = nullptr;
    float shopRadius=5;
    sf::CircleShape shopCircle(shopRadius);
    shopCircle.setFillColor(sf::Color::Yellow);

    // Create nodes
    //const int rows = 20;
    //const int cols = 20;
    //std::vector<Node> nodes = createNodes(rows, cols, window);
    std::vector<Node> nodes = loadNodes("maps/lion_city.map"); // Load nodes from file

    //create a vector of `Request` structures 
    std::vector<Request> requests;
    float requestProbability = 0.0001;  // Set the probability of a shop generating a delivery request

    // Create shape for packages
    std::vector<sf::RectangleShape> packages;
    sf::Clock animationClock;

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
        // Find the node nearest the cursor, in screen space, for edit-mode hover/selection.
        // Screen space keeps the pick radius feeling consistent regardless of zoom level,
        // and sidesteps the pixel-vs-world mismatch a raw pixel/world comparison would have.
        bool mouseOverSidebar = sf::Mouse::getPosition(window).x >= static_cast<int>(window.getSize().x - sidebarWidth);
        Node* hoverNode = nullptr;
        if (edit_mode && !mouseOverSidebar) {
            sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
            float closestDist = std::numeric_limits<float>::max();
            for (auto& node : nodes) {
                sf::Vector2i nodeScreen = window.mapCoordsToPixel(node.position, view);
                float dx = static_cast<float>(nodeScreen.x - mousePixel.x);
                float dy = static_cast<float>(nodeScreen.y - mousePixel.y);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < closestDist) {
                    closestDist = dist;
                    hoverNode = &node;
                }
            }
            if (closestDist > nodePickRadius) {
                hoverNode = nullptr;
            }
        }

        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                view = sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height));
                uiView = view;
                zoom = 1.f;
                window.setView(view);
            } else if (event.type == sf::Event::MouseWheelMoved) {
                // Zoom around the point under the cursor: capture its world position,
                // resize the view, then shift the view so that same world point is
                // back under the cursor (instead of jumping the center to the cursor).
                sf::Vector2f mouseWorldBefore = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
                delta = static_cast<float>(event.mouseWheel.delta);
                zoomFactor = std::pow(1.1f, delta);
                zoom = zoom * zoomFactor;
                view.setSize(window.getDefaultView().getSize() / zoom);
                window.setView(view);
                sf::Vector2f mouseWorldAfter = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
                view.move(mouseWorldBefore - mouseWorldAfter);
                window.setView(view);
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    panning = true;
                    panAnchorWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
                } else if (event.mouseButton.button == sf::Mouse::Left && edit_mode && hoverNode != nullptr) {
                    closestNode = hoverNode;
                    std::cout << "Selected node: position=(" << closestNode->position.x << "," << closestNode->position.y << ")" << std::endl;
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    panning = false;
                }
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
                    nodes = loadNodes("maps/lion_city.map"); // Load nodes from file
                }
            }
            if (edit_mode) {
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

        // Re-anchor the view every frame while middle-mouse dragging, so the world
        // point grabbed on press stays under the cursor as it moves.
        if (panning) {
            sf::Vector2f currentWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
            view.move(panAnchorWorld - currentWorld);
            window.setView(view);
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
        
        //generating a delivery request
        if (!runners.empty()) {
            for (auto& node : nodes) {
                if (node.has_shop && (rand() / float(RAND_MAX)) < requestProbability) {
                    Node* destinationNode = &nodes[rand() % nodes.size()];
                    Runner* designatedRunner = &runners[rand() % runners.size()];
                    requests.push_back({&node, designatedRunner, destinationNode, false, animationClock.getElapsedTime().asSeconds()});
                    // define the package drawable objects
                    sf::RectangleShape package(sf::Vector2f(10, 10)); // (width, height) of the rectangle
                    package.setPosition(node.position);
                    packages.push_back(package);
                }
            }
        }

        drawCity(window, nodes, nodeCircle, streets, shopCircle);
        drawRunners(window, runners);
        drawRequests(window, requests, packages, animationClock.getElapsedTime().asSeconds());

        if (edit_mode) {
            if (hoverNode != nullptr && hoverNode != closestNode) {
                nodeHover.setPosition(hoverNode->position - sf::Vector2f(nodeHover.getRadius(), nodeHover.getRadius()));
                window.draw(nodeHover);
            }
            if (closestNode != nullptr) {
                nodeSelect.setPosition(closestNode->position - sf::Vector2f(nodeSelect.getRadius(), nodeSelect.getRadius()));
                window.draw(nodeSelect);
            }
        }
        if (event.type == sf::Event::KeyPressed) {
            if (edit_mode and (sf::Keyboard::isKeyPressed(sf::Keyboard::G))) {
                saveNodes(nodes, "new_city.map");
            }
        }

        // Draw the sidebar HUD, fixed to the screen regardless of the world view's zoom/pan
        if (fontLoaded) {
            std::size_t numShops = 0;
            for (const auto& node : nodes) {
                if (node.has_shop) {
                    ++numShops;
                }
            }
            sf::Vector2f sidebarPosition(window.getSize().x - sidebarWidth, 0.f);
            sf::Vector2f sidebarSize(sidebarWidth, static_cast<float>(window.getSize().y));
            window.setView(uiView);
            drawSidebar(window, font, sidebarPosition, sidebarSize, edit_mode, runners.size(), numShops, requests.size());
            window.setView(view);
        }

        // Display the window
        window.display();
    }

    return 0;
}

