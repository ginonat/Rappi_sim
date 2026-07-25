#include "../include/struct.h"
#include <iostream>


// runners struct
Runner::Runner(Node* start_node, sf::Vector2f box_size, sf::Color box_color, float movement_speed)
    : current_node(start_node), target_node(start_node), movement_speed(movement_speed)
{
    box.setPosition(current_node->position);
    box.setSize(box_size);
    box.setFillColor(box_color);
}

void Runner::moveToNextNode() {
    if (!this->route.empty()) {
        Node* next = this->route.front();
        this->route.erase(this->route.begin());
        this->currentEdgeTier = findEdgeTier(this->current_node, next);
        this->target_node = next;
        this->running = true;
        return;
    }
    if (!this->current_node->edges.empty()) {
        std::vector<Edge>::size_type edgeIndex = rand() % this->current_node->edges.size();
        const Edge& edge = this->current_node->edges[edgeIndex];
        this->target_node = edge.target;
        this->currentEdgeTier = edge.tier;
        this->running = true;
    }
}

float edgeTierMultiplier(EdgeTier tier) {
    switch (tier) {
        case EdgeTier::Long: return 10.f;
        case EdgeTier::VeryLong: return 100.f;
        default: return 1.f;
    }
}

std::string edgeTierLabel(EdgeTier tier) {
    switch (tier) {
        case EdgeTier::Long: return "Long (10x)";
        case EdgeTier::VeryLong: return "Very Long (100x)";
        default: return "Normal (1x)";
    }
}

EdgeTier findEdgeTier(const Node* from, const Node* to) {
    for (const auto& edge : from->edges) {
        if (edge.target == to) {
            return edge.tier;
        }
    }
    return EdgeTier::Normal;
}
