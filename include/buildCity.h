#include <vector>

std::vector<Node> createNodes(int rows, int cols, sf::RenderWindow& window);

std::vector<Node> loadNodes(const std::string& filename);

void saveNodes(const std::vector<Node>& nodes, const std::string& filename);
