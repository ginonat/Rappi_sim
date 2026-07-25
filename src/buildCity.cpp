#include <iostream>
#include <limits>
#include <sstream>
#include <fstream>
#include <utility>
#include "../include/struct.h"
#include "../include/buildCity.h"

// Create nodes
std::deque<Node> createNodes(int rows, int cols, sf::RenderWindow& window) {
    const float nodeSpacing_x = window.getSize().x / (cols-1);
    const float nodeSpacing_y = window.getSize().y / (rows-1);
    std::deque<Node> nodes(rows * cols);
    int count=0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Node& node = nodes[i * cols + j];
            node.id=count;
            count=count+1;
            node.position = sf::Vector2f(j * nodeSpacing_x + (rand()%20) * nodeSpacing_x/300, i * nodeSpacing_y+ (rand()%20) * nodeSpacing_y/300);
            if (i > 0) {
                node.edges.push_back({&nodes[(i - 1) * cols + j], EdgeTier::Normal});
            }
            if (j > 0) {
                node.edges.push_back({&nodes[i * cols + (j - 1)], EdgeTier::Normal});
            }
            if (i < rows - 1) {
                node.edges.push_back({&nodes[(i + 1) * cols + j], EdgeTier::Normal});
            }
            if (j < cols - 1) {
                node.edges.push_back({&nodes[i * cols + (j + 1)], EdgeTier::Normal});
            }
        }
    }
    return nodes;
}

namespace {
Node* findNodeById(std::deque<Node>& nodes, int id) {
    for (auto& node : nodes) {
        if (node.id == id) {
            return &node;
        }
    }
    return nullptr;
}
}

std::deque<Node> loadNodes(const std::string& filename) {
    std::deque<Node> nodes;
    // Link ids/tiers parsed before all nodes exist, resolved into real Edge pointers once
    // the full node list is available. Indexed in lockstep with `nodes`.
    std::vector<std::vector<std::pair<int, EdgeTier>>> pendingLinks;

    std::ifstream file(filename);
    if (file.is_open()) {
        float x, y;
        bool has_shop;
        int id;
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            if (iss >> x >> y >> has_shop >> id) {
                Node node;
                node.position = sf::Vector2f(x, y);
                node.has_shop = has_shop;
                node.id = id;
                std::string links_prefix;
                iss >> links_prefix;  // Reads 'links:' part

                std::vector<std::pair<int, EdgeTier>> links;
                std::string token;
                while (iss >> token) {
                    std::size_t colon = token.find(':');
                    int neighborID;
                    EdgeTier tier = EdgeTier::Normal;
                    if (colon != std::string::npos) {
                        neighborID = std::stoi(token.substr(0, colon));
                        tier = static_cast<EdgeTier>(std::stoi(token.substr(colon + 1)));
                    } else {
                        // Old format, no tier suffix: plain neighbor id, defaults to Normal.
                        neighborID = std::stoi(token);
                    }
                    links.push_back({neighborID, tier});
                }
                nodes.push_back(node);
                pendingLinks.push_back(links);
            }
        }
        file.close();
        std::cout << "Nodes loaded from " << filename << std::endl;
    } else {
        std::cout << "Unable to open file " << filename << std::endl;
    }

    std::size_t idx = 0;
    for (auto& node : nodes) {
        for (const auto& link : pendingLinks[idx]) {
            Node* target = findNodeById(nodes, link.first);
            if (target != nullptr) {
                node.edges.push_back({target, link.second});
            }
        }
        ++idx;
    }

    return nodes;
}


void saveNodes(const std::deque<Node>& nodes, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& node : nodes) {
            file << node.position.x << " " << node.position.y << " " << node.has_shop << " " << node.id << ";links: ";
            for (const auto& edge : node.edges){
                file << edge.target->id << ":" << static_cast<int>(edge.tier) << " ";
            }
            file << "\n";
        }
        file.close();
        std::cout << "Nodes saved to " << filename << std::endl;
    } else {
        std::cout << "Unable to open file " << filename << std::endl;
    }
}
