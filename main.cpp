#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath> 
struct Node {
    sf::Vector2f position;
    std::vector<Node*> neighbors;
};


void drawArrows(sf::RenderWindow& window, Node* node) {
    const sf::Color lineColor = sf::Color::White; // color of the lines
    const float arrowSize = 10.f; // size of the arrowhead
    const float pi = 3.14159265358979323846f; // the value of pi
    
    // Load a font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        // handle error if the font file could not be loaded
    }
    
    // Iterate through the node's neighbors and draw lines to them
    for (std::vector<Node*>::size_type i = 0; i < node->neighbors.size(); i++) {


        // Get the current neighbor
        const auto& neighbor = node->neighbors[i];
        
        // calculate the direction of the line
        sf::Vector2f dir = neighbor->position - node->position;
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir /= length;
        
        // calculate the midpoint between the node and its neighbor
        sf::Vector2f midpoint = node->position + dir * length / 2.f;
        
        // draw the line
        sf::Vertex line[] = {
            sf::Vertex(node->position, lineColor),
            sf::Vertex(neighbor->position, lineColor)
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);

        // calculate the angle of the arrowhead
        float angle = std::atan2(dir.y, dir.x);
        sf::Transform transform;
        transform.rotate(angle * 180.f / pi);
        
        // draw the arrowhead
        sf::ConvexShape arrow;
        arrow.setPointCount(3);
        arrow.setPoint(0, sf::Vector2f(0.f, -arrowSize / 2.f));
        arrow.setPoint(1, sf::Vector2f(-arrowSize, arrowSize / 2.f));
        arrow.setPoint(2, sf::Vector2f(0.f, arrowSize / 4.f));
        arrow.setFillColor(lineColor);
        arrow.setOutlineThickness(0.f);
        arrow.setPosition(neighbor->position);
        arrow.setOrigin(sf::Vector2f(arrowSize / 2.f, 0.f));
        arrow.setRotation(angle * 180.f / pi);
        window.draw(arrow);

        // create a text object to display the index
        sf::Text text(std::to_string(i), sf::Font(), 16);
        // draw the neighbor index in the midpoint
        text.setFont(font);
        text.setString(std::to_string(i));
        text.setCharacterSize(14);
        text.setFillColor(lineColor);
        text.setOutlineColor(lineColor);
        text.setOutlineThickness(0.1f);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f);
        text.setPosition(midpoint);
        window.draw(text);
    }
}
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






    // runners struct
    struct Runner {
        sf::RectangleShape box;
        Node* current_node;
        float movement_speed = 0.01f;
    
        Runner(Node* start_node, sf::Vector2f box_size = sf::Vector2f(10, 10), sf::Color box_color = sf::Color::Red, float movement_speed = 0.01f)
            : current_node(start_node), movement_speed(movement_speed)
   
        {
            box.setPosition(current_node->position);
            box.setSize(box_size);
            box.setFillColor(box_color);
        }

        void moveToNextNode(long unsigned int neighborIndex) {
            if (!current_node->neighbors.empty()) {
                if (neighborIndex >= 0 && neighborIndex < current_node->neighbors.size()) {
                    current_node = current_node->neighbors[neighborIndex];
                    box.setPosition(current_node->position);
                }
            }
        }
    };
    
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
            if (event.type == sf::Event::KeyReleased) {
                // Check if the key released is the right arrow key
                if (event.key.code == sf::Keyboard::Right) {
                    // Change the current node of runner_A to one of its neighbors
                    runner_A.moveToNextNode(1); // move to the second neighbor of the current node
                }
                // Check if the key released is the left arrow key
                if (event.key.code == sf::Keyboard::Left) {
                    // Change the current node of runner_A to one of its neighbors
                    runner_A.moveToNextNode(0); // move to the first neighbor of the current node
                }
                std::cout << "Runner A moved to node at position (" << runner_A.current_node->neighbors[1]->position.x << ", " << runner_A.current_node->neighbors[1]->position.y << ")" << std::endl;
            }
        }



        // Clear the window
        window.clear(sf::Color::Black);


        // Draw the Nodes
        for (const auto& node : nodes_sprit) {
            window.draw(node);
            drawArrows(window, runner_A.current_node);
        }
        // Draw streets
//        window.draw(streets.data(), streets.size(), sf::streets);
        window.draw(streets);

        // Draw the runner
        window.draw(runner_A.box);

        // Display the window
        window.display();
    }

    return 0;
}

