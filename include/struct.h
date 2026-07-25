#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// Forward declarations
struct Node;
struct Request;

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

    std::vector<Node*> route;         // remaining nodes to visit for the current leg
    Request* activeRequest = nullptr; // request being fulfilled, nullptr when idle
    bool hasPackage = false;          // false: heading to the shop, true: carrying the package

    Runner(Node* start_node, sf::Vector2f box_size = sf::Vector2f(10, 10), sf::Color box_color = sf::Color::Red, float movement_speed = 1.0f);

    void moveToNextNode();
};

struct Request
{
    Node* holder;
    Runner* designatedRunner;
    Node* destination;
    bool satisfied;
    float spawnTime;
};

// Node struct
struct Node {
    int id;
    sf::Vector2f position;
    std::vector<Node*> neighbors;
    std::vector<int> neighborsID;
    bool has_shop = false;
};
