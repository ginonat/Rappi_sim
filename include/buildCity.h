#include <list>

std::list<Node> createNodes(int rows, int cols, sf::FloatRect bounds, int idOffset = 0);

std::list<Node> loadNodes(const std::string& filename);

void saveNodes(const std::list<Node>& nodes, const std::string& filename);
