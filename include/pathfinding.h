#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include "struct.h"

// Shortest path from start to goal by street distance (Dijkstra), inclusive of both
// endpoints. Returns an empty vector if goal is unreachable from start.
std::vector<Node*> findPath(Node* start, Node* goal);

#endif // PATHFINDING_H
