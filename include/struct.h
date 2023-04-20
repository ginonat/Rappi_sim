#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Forward declaration of Node struct
struct Node;

// Runner struct
struct Runner {
    sf::RectangleShape box;
    Node* current_node;
    Node* target_node;
    float movement_speed;

    Runner(Node* start_node, sf::Vector2f box_size = sf::Vector2f(10, 10), sf::Color box_color = sf::Color::Red, float movement_speed = 1.0f);

    void moveToNextNode(long unsigned int neighborIndex);
};

// Node struct
struct Node {
    sf::Vector2f position;
    std::vector<Node*> neighbors;
};
