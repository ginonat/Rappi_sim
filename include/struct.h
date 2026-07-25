#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Forward declarations
struct Node;
struct Request;

// How much longer a street connection actually is than its on-screen length suggests.
// Drawn as a distinct color per tier and crossed proportionally slower, rather than
// literally stretching the map 10x/100x wider.
enum class EdgeTier { Normal = 0, Long = 1, VeryLong = 2 };

struct Edge {
    Node* target = nullptr;
    EdgeTier tier = EdgeTier::Normal;
};

float edgeTierMultiplier(EdgeTier tier);
std::string edgeTierLabel(EdgeTier tier);
EdgeTier findEdgeTier(const Node* from, const Node* to);

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
    EdgeTier currentEdgeTier = EdgeTier::Normal; // tier of the edge currently being crossed

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
    float itemCost = 0.f;     // random, bell-curve-distributed cost of the good itself
    float logisticsFee = 0.f; // rate * distance actually travelled, accumulated leg by leg
};

// Node struct
struct Node {
    int id;
    sf::Vector2f position;
    std::vector<Edge> edges;
    bool has_shop = false;
};
