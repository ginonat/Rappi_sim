#include <deque>
#include <iostream>
#include <limits>
#include <list>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <fstream>

#include "../include/draw.h"
#include "../include/struct.h"
#include "../include/buildCity.h"
#include "../include/pathfinding.h"

// Nearest runner (by live position) that isn't already fulfilling a request.
Runner* findNearestIdleRunner(std::deque<Runner>& runners, const sf::Vector2f& position) {
    Runner* nearest = nullptr;
    float bestDist = std::numeric_limits<float>::max();
    for (auto& runner : runners) {
        if (runner.activeRequest != nullptr) {
            continue;
        }
        sf::Vector2f diff = runner.box.getPosition() - position;
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (dist < bestDist) {
            bestDist = dist;
            nearest = &runner;
        }
    }
    return nearest;
}

// Points the runner toward `goal`, dropping the node it's effectively standing on (or
// heading to, if it's mid-step) since it doesn't need to be told to walk to itself.
void routeRunnerTo(Runner& runner, Node* goal) {
    Node* effectiveStart = runner.running ? runner.target_node : runner.current_node;
    std::vector<Node*> path = findPath(effectiveStart, goal);
    if (!path.empty()) {
        path.erase(path.begin());
    }
    runner.route = path;
}

void dispatchRunnerToShop(Runner& runner, Request& request) {
    runner.activeRequest = &request;
    request.designatedRunner = &runner;
    runner.hasPackage = false;
    routeRunnerTo(runner, request.holder);
}

void dispatchRunnerToDestination(Runner& runner) {
    runner.hasPackage = true;
    routeRunnerTo(runner, runner.activeRequest->destination);
}

float sliderValueFromMouseX(int mouseX, float sliderLeft, float sliderRight, float minValue, float maxValue) {
    float t = (static_cast<float>(mouseX) - sliderLeft) / (sliderRight - sliderLeft);
    t = std::max(0.f, std::min(1.f, t));
    return minValue + t * (maxValue - minValue);
}

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

    // runner speed slider: a global multiplier applied on top of each runner's own
    // movement_speed, so it can be adjusted live without touching already-spawned runners
    const float sliderMargin = 10.f;
    const float sliderOffsetY = 300.f; // from the top of the sidebar, clear of the stats text
    const float speedMin = 0.2f;
    const float speedMax = 5.f;
    float speedMultiplier = 1.f;
    bool draggingSlider = false;

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

    //create a list of `Request` structures. A list, not a vector/deque, because runners hold
    //long-lived pointers into it (Runner::activeRequest) that must keep working even after
    //satisfied requests are erased from the middle - a list never invalidates references to
    //elements other than the one actually erased.
    std::list<Request> requests;
    float requestProbability = 0.0001;  // Set the probability of a shop generating a delivery request

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
    
    // Create a deque to store the red runners. A deque, not a vector, because requests hold
    // long-lived pointers into it (Request::designatedRunner) that must survive later pushes.
    std::deque<Runner> runners;


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
                } else if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
                    float sliderLeft = window.getSize().x - sidebarWidth + sliderMargin;
                    float sliderRight = window.getSize().x - sliderMargin;
                    // The track itself sits 22px below sliderOffsetY (see drawSpeedSlider);
                    // this hit-box is generous on both sides of it for easy grabbing.
                    float sliderTop = sliderOffsetY + 12.f;
                    float sliderBottom = sliderOffsetY + 32.f;
                    if (mousePixel.x >= sliderLeft && mousePixel.x <= sliderRight &&
                        mousePixel.y >= sliderTop && mousePixel.y <= sliderBottom) {
                        draggingSlider = true;
                        speedMultiplier = sliderValueFromMouseX(mousePixel.x, sliderLeft, sliderRight, speedMin, speedMax);
                    } else if (edit_mode && hoverNode != nullptr) {
                        closestNode = hoverNode;
                        std::cout << "Selected node: position=(" << closestNode->position.x << "," << closestNode->position.y << ")" << std::endl;
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    panning = false;
                } else if (event.mouseButton.button == sf::Mouse::Left) {
                    draggingSlider = false;
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

        if (draggingSlider) {
            float sliderLeft = window.getSize().x - sidebarWidth + sliderMargin;
            float sliderRight = window.getSize().x - sliderMargin;
            speedMultiplier = sliderValueFromMouseX(sf::Mouse::getPosition(window).x, sliderLeft, sliderRight, speedMin, speedMax);
        }

        if (!edit_mode) {
            // Check if the event is a keyPressed
            for (auto& runner : runners) {
                // Check if the runner has reached its target node
                if (runner.running && runner.box.getPosition() == runner.target_node->position) {
                    runner.current_node = runner.target_node;
                    runner.running = false;
                }
                // Arrived with nowhere left to go on this leg: if it's fulfilling a
                // request, that means either "reached the shop" (pick up, route to the
                // destination next) or "reached the destination" (deliver, go idle).
                if (!runner.running && runner.route.empty() && runner.activeRequest != nullptr) {
                    if (!runner.hasPackage) {
                        dispatchRunnerToDestination(runner);
                    } else {
                        runner.activeRequest->satisfied = true;
                        runner.activeRequest = nullptr;
                        runner.hasPackage = false;
                    }
                }
                // If the runner is not currently running, select a new target node
                if (!runner.running) {
                    runner.moveToNextNode();
                }
            }
        }

        // Drop satisfied requests now that nothing points at them any more - the runner
        // that fulfilled each one already cleared its own activeRequest above, in the same
        // frame, so no dangling Runner::activeRequest is left behind by the erase.
        requests.remove_if([](const Request& request) { return request.satisfied; });

        // Clear the window
        window.clear(sf::Color::Black);

        // Interpolate runner position towards target node
        for (auto& runner : runners) {
            sf::Vector2f distance_to_target = runner.target_node->position - runner.box.getPosition();
            float distance = std::sqrt(distance_to_target.x * distance_to_target.x + distance_to_target.y * distance_to_target.y);
            if (distance > 0) {
                float step = runner.movement_speed * speedMultiplier;
                sf::Vector2f direction = distance_to_target / distance;
                sf::Vector2f velocity = direction * step;
                sf::Vector2f new_position = runner.box.getPosition() + velocity;
                if (distance < step) {
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
                    requests.push_back({&node, nullptr, destinationNode, false, animationClock.getElapsedTime().asSeconds()});
                }
            }
        }

        // Dispatch the nearest idle runner to any request that doesn't have one yet -
        // freshly created ones, or ones still waiting because every runner was busy.
        for (auto& request : requests) {
            if (!request.satisfied && request.designatedRunner == nullptr) {
                Runner* nearest = findNearestIdleRunner(runners, request.holder->position);
                if (nearest != nullptr) {
                    dispatchRunnerToShop(*nearest, request);
                }
            }
        }

        drawCity(window, nodes, nodeCircle, streets, shopCircle);
        drawRunners(window, runners);
        drawRequests(window, requests, animationClock.getElapsedTime().asSeconds());

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
            drawSpeedSlider(window, font, sf::Vector2f(sidebarPosition.x + sliderMargin, sidebarPosition.y + sliderOffsetY),
                             sidebarWidth - 2.f * sliderMargin, speedMin, speedMax, speedMultiplier);
            window.setView(view);
        }

        // Display the window
        window.display();
    }

    return 0;
}

