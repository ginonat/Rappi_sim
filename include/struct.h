#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Forward declaration of Node struct
struct Node;

// Shop structure
struct Shop {
    sf::Vector2f position;
};

// Runner struct
struct Runner {
    sf::RectangleShape box;
    Node* current_node;
    Node* target_node;
    float movement_speed;
    bool running =false;

    Runner(Node* start_node, sf::Vector2f box_size = sf::Vector2f(10, 10), sf::Color box_color = sf::Color::Red, float movement_speed = 1.0f);

    void moveToNextNode();
};

// Node struct
struct Node {
    int id;
    sf::Vector2f position;
    std::vector<Node*> neighbors;
    std::vector<int> neighborsID;
    bool has_shop = false;
};
