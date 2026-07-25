#include "../include/pathfinding.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <utility>

namespace {
float distance(const sf::Vector2f& a, const sf::Vector2f& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
}

std::vector<Node*> findPath(Node* start, Node* goal) {
    if (start == nullptr || goal == nullptr) {
        return {};
    }
    if (start == goal) {
        return {start};
    }

    std::unordered_map<Node*, float> bestDistance;
    std::unordered_map<Node*, Node*> cameFrom;
    typedef std::pair<float, Node*> QueueEntry;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry> > frontier;

    bestDistance[start] = 0.f;
    frontier.push(std::make_pair(0.f, start));

    while (!frontier.empty()) {
        float currentDist = frontier.top().first;
        Node* current = frontier.top().second;
        frontier.pop();

        if (current == goal) {
            break;
        }
        if (currentDist > bestDistance[current]) {
            continue; // a shorter route to this node was already processed
        }

        for (const Edge& edge : current->edges) {
            Node* neighbor = edge.target;
            float newDist = currentDist + distance(current->position, neighbor->position) * edgeTierMultiplier(edge.tier);
            std::unordered_map<Node*, float>::iterator it = bestDistance.find(neighbor);
            if (it == bestDistance.end() || newDist < it->second) {
                bestDistance[neighbor] = newDist;
                cameFrom[neighbor] = current;
                frontier.push(std::make_pair(newDist, neighbor));
            }
        }
    }

    if (cameFrom.find(goal) == cameFrom.end()) {
        return {}; // goal is unreachable from start
    }

    std::vector<Node*> path;
    Node* current = goal;
    path.push_back(current);
    while (current != start) {
        current = cameFrom[current];
        path.push_back(current);
    }
    std::reverse(path.begin(), path.end());
    return path;
}
