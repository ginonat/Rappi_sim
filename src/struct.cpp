#include "../include/struct.h"


// runners struct
Runner::Runner(Node* start_node, sf::Vector2f box_size, sf::Color box_color, float movement_speed)
    : current_node(start_node), target_node(start_node), movement_speed(movement_speed)
{
    box.setPosition(current_node->position);
    box.setSize(box_size);
    box.setFillColor(box_color);
}

void Runner::moveToNextNode(long unsigned int neighborIndex) {
    if (!this->current_node->neighbors.empty()) {
        if (neighborIndex >= 0 && neighborIndex < this->current_node->neighbors.size()) {
            this->target_node  = this->current_node->neighbors[neighborIndex];
            this->running      = true;
        }
    }
}


