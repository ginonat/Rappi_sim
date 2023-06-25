#include  "../include/draw.h"
void drawCity(sf::RenderWindow& window, const std::vector<Node>& nodes, sf::CircleShape nodeCircle, const sf::VertexArray& streets, sf::CircleShape& shopCircle) {
    for (const auto& node : nodes) {
        if(node.has_shop){
            shopCircle.setPosition(node.position);
            window.draw(shopCircle);
        } else{
            nodeCircle.setPosition(node.position);
            window.draw(nodeCircle);
        }
    }
    //Draw streets
    window.draw(streets);
}


void drawRunners(sf::RenderWindow& window, std::vector<Runner>& runners) {
    for (auto& runner : runners) {
        // Draw the runner
        window.draw(runner.box);
    }
}


