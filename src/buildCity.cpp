#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>
#include "../include/struct.h"
#include "../include/buildCity.h"

// Create nodes
std::vector<Node> createNodes(int rows, int cols, sf::RenderWindow& window) {
    const float nodeSpacing_x = window.getSize().x / (cols-1);
    const float nodeSpacing_y = window.getSize().y / (rows-1);
    std::vector<Node> nodes(rows * cols);
    int count=0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Node& node = nodes[i * cols + j];
            node.id=count;
            count=count+1;
            node.position = sf::Vector2f(j * nodeSpacing_x + (rand()%20) * nodeSpacing_x/300, i * nodeSpacing_y+ (rand()%20) * nodeSpacing_y/300);
            if (i > 0) {
                node.neighbors.push_back(&nodes[(i - 1) * cols + j]);
            }
            if (j > 0) {
                node.neighbors.push_back(&nodes[i * cols + (j - 1)]);
            }
            if (i < rows - 1) {
                node.neighbors.push_back(&nodes[(i + 1) * cols + j]);
            }
            if (j < cols - 1) {
                node.neighbors.push_back(&nodes[i * cols + (j + 1)]);
            }
        }
    }
    return nodes;
}

Node* finde_node(int search_id, std::vector<Node> nodes){
        for ( auto& node : nodes) {
            if (node.id==search_id){
                return &node;
            }
        }
    return 0;
}

std::vector<Node*> getNeighbors(Node& node, std::vector<Node>& allNodes) {
    std::vector<Node*> neighborNodes;
    for(auto& id : node.neighborsID) {
        for(auto& n : allNodes) {
            if(n.id == id) {
                neighborNodes.push_back(&n);
                break; // once the node is found, no need to look further
            }
        }
    }
    return neighborNodes;
}

std::vector<Node> loadNodes(const std::string& filename) {
    std::vector<Node> nodes;
    std::ifstream file(filename);
    if (file.is_open()) {
        float x, y;
        bool has_shop;
        int id;
        int neighborID;
        std::string line;
        while (std::getline(file, line)) {
            neighborID=-1;
            std::istringstream iss(line);
            if (iss >> x >> y >> has_shop >> id) {
                Node node;
                node.position = sf::Vector2f(x, y);
                node.has_shop = has_shop;
                node.id = id;
                std::string links_prefix;
                iss >> links_prefix;  // Reads 'links:' part
                while (iss >> neighborID) {
                    node.neighborsID.push_back(neighborID);
                }
                nodes.push_back(node);
            }
        }
        file.close();
        std::cout << "Nodes loaded from " << filename << std::endl;
    } else {
        std::cout << "Unable to open file " << filename << std::endl;
    }

    for (auto& node : nodes) {
        node.neighbors=getNeighbors(node, nodes);
    }

    return nodes;
}


void saveNodes(const std::vector<Node>& nodes, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& node : nodes) {
            file << node.position.x << " " << node.position.y << " " << node.has_shop << " " << node.id << ";links: ";
            for (const auto& neighbor : node.neighbors){
                file << neighbor->id << " ";
            }
            file << "\n";
        }
        file.close();
        std::cout << "Nodes saved to " << filename << std::endl;
    } else {
        std::cout << "Unable to open file " << filename << std::endl;
    }
}
