#include <algorithm>
#include <deque>
#include <dirent.h>
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

enum class AppState { StartMenu, Running };

// Every ".map" file directly inside `directory`, sorted for a stable menu order.
std::vector<std::string> listMapFiles(const std::string& directory) {
    std::vector<std::string> paths;
    DIR* dir = opendir(directory.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name.size() > 4 && name.substr(name.size() - 4) == ".map") {
                paths.push_back(directory + "/" + name);
            }
        }
        closedir(dir);
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

// "maps/lion_city.map" -> "lion_city"
std::string mapDisplayName(const std::string& path) {
    std::size_t slash = path.find_last_of('/');
    std::size_t start = (slash == std::string::npos) ? 0 : slash + 1;
    std::size_t dot = path.find_last_of('.');
    std::size_t end = (dot == std::string::npos || dot < start) ? path.size() : dot;
    return path.substr(start, end - start);
}

sf::Color colorForTier(EdgeTier tier) {
    switch (tier) {
        case EdgeTier::Long: return sf::Color(255, 165, 0);   // orange
        case EdgeTier::VeryLong: return sf::Color(255, 60, 60); // red
        default: return sf::Color::White;
    }
}

sf::VertexArray buildStreets(const std::deque<Node>& nodes) {
    sf::VertexArray streets(sf::Lines, 0);
    for (const auto& node : nodes) {
        for (const auto& edge : node.edges) {
            sf::Color color = colorForTier(edge.tier);
            streets.append(sf::Vertex(node.position, color));
            streets.append(sf::Vertex(edge.target->position, color));
        }
    }
    return streets;
}

// Loads a city and discards all in-flight simulation state. Necessary, not just tidy:
// runners/requests hold raw Node*/Runner*/Request* pointers into these containers, and
// replacing `nodes` invalidates every one of them.
void loadCityAndResetSimulation(const std::string& path, std::deque<Node>& nodes, sf::VertexArray& streets,
                                 std::deque<Runner>& runners, std::list<Request>& requests, Node*& closestNode) {
    nodes = loadNodes(path);
    streets = buildStreets(nodes);
    runners.clear();
    requests.clear();
    closestNode = nullptr;
}

// Same reset semantics as loadCityAndResetSimulation, but for a freshly generated grid
// rather than one read from disk.
void generateGridAndResetSimulation(int gridSize, sf::RenderWindow& window, std::deque<Node>& nodes, sf::VertexArray& streets,
                                     std::deque<Runner>& runners, std::list<Request>& requests, Node*& closestNode) {
    nodes = createNodes(gridSize, gridSize, window);
    streets = buildStreets(nodes);
    runners.clear();
    requests.clear();
    closestNode = nullptr;
}

sf::Vector2f snapToGrid(sf::Vector2f pos, float size) {
    return sf::Vector2f(std::round(pos.x / size) * size, std::round(pos.y / size) * size);
}

sf::FloatRect computeLoadMapButtonBounds(const sf::RenderWindow& window) {
    float width = 240.f;
    float height = 44.f;
    float centerX = window.getSize().x / 2.f;
    float y = window.getSize().y / 2.f - 60.f;
    return sf::FloatRect(centerX - width / 2.f, y, width, height);
}

std::vector<sf::FloatRect> computeMapButtonBounds(const sf::RenderWindow& window, std::size_t count) {
    std::vector<sf::FloatRect> bounds;
    sf::FloatRect loadButton = computeLoadMapButtonBounds(window);
    float height = 38.f;
    float y = loadButton.top + loadButton.height + 16.f;
    for (std::size_t i = 0; i < count; ++i) {
        bounds.push_back(sf::FloatRect(loadButton.left, y, loadButton.width, height));
        y += height + 8.f;
    }
    return bounds;
}

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
    sf::RenderWindow window(sf::VideoMode(640, 480), "Rappi_sim", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.clear(sf::Color::Black);

    // create a view object
    sf::View view(window.getDefaultView());
    window.setView(view);

    // fixed-to-screen view for the sidebar HUD, unaffected by zoom/pan
    sf::View uiView(window.getDefaultView());
    const float sidebarWidth = 260.f; // wide enough for the longer editor-control lines

    // runner speed slider: a global multiplier applied on top of each runner's own
    // movement_speed, so it can be adjusted live without touching already-spawned runners
    const float sliderMargin = 10.f;
    const float sliderOffsetY = 520.f; // from the top of the sidebar, clear of the (longer, in edit mode) text
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
    //std::deque<Node> nodes = createNodes(rows, cols, window);
    // Left empty until a map is chosen from the start menu.
    std::deque<Node> nodes;

    // Editor state: tier used for newly-connected streets, grid-snap toggle, and the
    // pending size for a bulk-generated grid (rows == cols == gridSize).
    EdgeTier currentTier = EdgeTier::Normal;
    bool gridSnapEnabled = false;
    const float gridSnapSize = 25.f;
    int gridSize = 5;
    bool draggingNode = false;

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

    // Streets are derived from nodes; empty until a map is loaded, rebuilt by
    // loadCityAndResetSimulation whenever one is.
    sf::VertexArray streets = buildStreets(nodes);

    // Create a deque to store the red runners. A deque, not a vector, because requests hold
    // long-lived pointers into it (Request::designatedRunner) that must survive later pushes.
    std::deque<Runner> runners;

    // Start menu / map picker state
    AppState appState = AppState::StartMenu;
    std::vector<std::string> availableMaps = listMapFiles("maps");
    bool showMapList = false;
    std::string currentMapPath = "maps/lion_city.map"; // what `L` reloads while running
    if (!fontLoaded) {
        // Without a font the menu's labels can't render usably - skip straight to a
        // playable default rather than leaving the app stuck on an unreadable screen.
        loadCityAndResetSimulation(currentMapPath, nodes, streets, runners, requests, closestNode);
        appState = AppState::Running;
    }

    // Run the game loop
    while (window.isOpen()) {
        // Find the node nearest the cursor, in screen space, for edit-mode hover/selection.
        // Screen space keeps the pick radius feeling consistent regardless of zoom level,
        // and sidesteps the pixel-vs-world mismatch a raw pixel/world comparison would have.
        bool mouseOverSidebar = sf::Mouse::getPosition(window).x >= static_cast<int>(window.getSize().x - sidebarWidth);
        Node* hoverNode = nullptr;
        if (appState == AppState::Running && edit_mode && !mouseOverSidebar) {
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
            } else if (appState == AppState::StartMenu) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
                    sf::Vector2f mouse(static_cast<float>(mousePixel.x), static_cast<float>(mousePixel.y));
                    sf::FloatRect loadMapButton = computeLoadMapButtonBounds(window);
                    if (loadMapButton.contains(mouse)) {
                        showMapList = !showMapList;
                    } else if (showMapList) {
                        std::vector<sf::FloatRect> mapButtons = computeMapButtonBounds(window, availableMaps.size());
                        for (std::size_t i = 0; i < mapButtons.size(); ++i) {
                            if (mapButtons[i].contains(mouse)) {
                                currentMapPath = availableMaps[i];
                                loadCityAndResetSimulation(currentMapPath, nodes, streets, runners, requests, closestNode);
                                appState = AppState::Running;
                                showMapList = false;
                            }
                        }
                    }
                }
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
                    bool onSlider = mousePixel.x >= sliderLeft && mousePixel.x <= sliderRight &&
                                     mousePixel.y >= sliderTop && mousePixel.y <= sliderBottom;
                    if (onSlider) {
                        draggingSlider = true;
                        speedMultiplier = sliderValueFromMouseX(mousePixel.x, sliderLeft, sliderRight, speedMin, speedMax);
                    } else if (edit_mode && !mouseOverSidebar) {
                        bool shiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
                                         sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
                        if (shiftHeld && closestNode != nullptr && hoverNode != nullptr && hoverNode != closestNode) {
                            // Toggle the connection between the selected node and the one just clicked.
                            auto it = std::find_if(closestNode->edges.begin(), closestNode->edges.end(),
                                                    [&](const Edge& e) { return e.target == hoverNode; });
                            if (it != closestNode->edges.end()) {
                                closestNode->edges.erase(it);
                                auto it2 = std::find_if(hoverNode->edges.begin(), hoverNode->edges.end(),
                                                         [&](const Edge& e) { return e.target == closestNode; });
                                if (it2 != hoverNode->edges.end()) {
                                    hoverNode->edges.erase(it2);
                                }
                            } else {
                                closestNode->edges.push_back({hoverNode, currentTier});
                                hoverNode->edges.push_back({closestNode, currentTier});
                            }
                            streets = buildStreets(nodes);
                        } else if (hoverNode != nullptr) {
                            closestNode = hoverNode;
                            draggingNode = true;
                            std::cout << "Selected node: position=(" << closestNode->position.x << "," << closestNode->position.y << ")" << std::endl;
                        } else if (!shiftHeld) {
                            // Empty space, no modifier: drop a new, isolated node here.
                            sf::Vector2f worldPos = window.mapPixelToCoords(mousePixel, view);
                            if (gridSnapEnabled) {
                                worldPos = snapToGrid(worldPos, gridSnapSize);
                            }
                            int newId = 0;
                            for (const auto& n : nodes) {
                                newId = std::max(newId, n.id + 1);
                            }
                            Node newNode;
                            newNode.id = newId;
                            newNode.position = worldPos;
                            nodes.push_back(newNode);
                            closestNode = &nodes.back();
                        }
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    panning = false;
                } else if (event.mouseButton.button == sf::Mouse::Left) {
                    draggingSlider = false;
                    draggingNode = false;
                }
            }

            if (appState == AppState::Running) {
                // check for edit mode in every event keyPressed
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::E) {
                        edit_mode = !edit_mode;
                        draggingNode = false;
                        if (edit_mode) {
                            window.setTitle("Paused: Edit mode on");
                        } else {
                            window.setTitle("Unpaused: Edit mode off");
                        }
                    } else if (edit_mode && event.key.code == sf::Keyboard::L) {
                        loadCityAndResetSimulation(currentMapPath, nodes, streets, runners, requests, closestNode);
                    } else if (edit_mode && event.key.code == sf::Keyboard::G) {
                        saveNodes(nodes, "maps/new_city.map");
                    } else if (edit_mode && event.key.code == sf::Keyboard::Num1) {
                        currentTier = EdgeTier::Normal;
                    } else if (edit_mode && event.key.code == sf::Keyboard::Num2) {
                        currentTier = EdgeTier::Long;
                    } else if (edit_mode && event.key.code == sf::Keyboard::Num3) {
                        currentTier = EdgeTier::VeryLong;
                    } else if (edit_mode && event.key.code == sf::Keyboard::N) {
                        gridSnapEnabled = !gridSnapEnabled;
                    } else if (edit_mode && event.key.code == sf::Keyboard::C) {
                        generateGridAndResetSimulation(gridSize, window, nodes, streets, runners, requests, closestNode);
                    } else if (edit_mode && (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal)) {
                        gridSize = std::min(gridSize + 1, 20);
                    } else if (edit_mode && (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash)) {
                        gridSize = std::max(gridSize - 1, 2);
                    }
                }
                if (edit_mode) {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) and closestNode!=nullptr) {
                        closestNode->has_shop = true;
                    }
                } else {
                    if (event.type == sf::Event::KeyPressed) {
                        // Add new runner
                        if (event.key.code == sf::Keyboard::Add) {
                            Node* start_node = &nodes[rand() % nodes.size()];
                            runners.emplace_back(start_node, sf::Vector2f(10, 10), sf::Color::Blue, 0.03f);
                        }
                    }
                }
            }
        }

        if (appState != AppState::Running) {
            window.clear(sf::Color::Black);
            if (fontLoaded) {
                sf::FloatRect loadMapButton = computeLoadMapButtonBounds(window);
                std::vector<sf::FloatRect> mapButtons = computeMapButtonBounds(window, availableMaps.size());
                std::vector<std::string> mapLabels;
                for (const auto& path : availableMaps) {
                    mapLabels.push_back(mapDisplayName(path));
                }
                drawStartMenu(window, font, loadMapButton, mapButtons, mapLabels, showMapList, sf::Mouse::getPosition(window));
            }
            window.display();
            continue;
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

        if (draggingNode && closestNode != nullptr) {
            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);
            if (gridSnapEnabled) {
                worldPos = snapToGrid(worldPos, gridSnapSize);
            }
            closestNode->position = worldPos;
            streets = buildStreets(nodes);
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
                float step = runner.movement_speed * speedMultiplier / edgeTierMultiplier(runner.currentEdgeTier);
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
            drawSidebar(window, font, sidebarPosition, sidebarSize, edit_mode, runners.size(), numShops, requests.size(),
                        edgeTierLabel(currentTier), gridSnapEnabled, gridSize);
            drawSpeedSlider(window, font, sf::Vector2f(sidebarPosition.x + sliderMargin, sidebarPosition.y + sliderOffsetY),
                             sidebarWidth - 2.f * sliderMargin, speedMin, speedMax, speedMultiplier);
            window.setView(view);
        }

        // Display the window
        window.display();
    }

    return 0;
}

