#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include "struct.h"

// Shortest path from start to goal by street distance (Dijkstra), inclusive of both
// endpoints. Returns an empty vector if goal is unreachable from start.
std::vector<Node*> findPath(Node* start, Node* goal);

// Total tiered distance along a path returned by findPath - the same cost Dijkstra
// minimized, recomputed from the node sequence. 0 for a path of 0 or 1 nodes.
float pathDistance(const std::vector<Node*>& path);

#endif // PATHFINDING_H
