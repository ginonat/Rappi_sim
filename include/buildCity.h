#include <deque>

std::deque<Node> createNodes(int rows, int cols, sf::RenderWindow& window);

std::deque<Node> loadNodes(const std::string& filename);

void saveNodes(const std::deque<Node>& nodes, const std::string& filename);
