#include  "../include/draw.h"
void drawCity(sf::RenderWindow& window, const std::vector<Node>& nodes, sf::CircleShape nodeCircle, const sf::VertexArray& streets, sf::CircleShape& shopCircle) {
    for (const auto& node : nodes) {
        if(node.has_shop){
            shopCircle.setPosition(node.position);
            shopCircle.setPosition(node.position - sf::Vector2f(shopCircle.getRadius(), shopCircle.getRadius()));
            window.draw(shopCircle);
        } else{
            nodeCircle.setPosition(node.position);
            nodeCircle.setPosition(node.position - sf::Vector2f(nodeCircle.getRadius(), nodeCircle.getRadius()));
            window.draw(nodeCircle);
        }
    }
    //Draw streets
    window.draw(streets);
}


void drawRunners(sf::RenderWindow& window, std::vector<Runner>& runners) {
    for (auto& runner : runners) {
        // Draw the runner
        runner.box.setPosition(runner.box.getPosition() - sf::Vector2f(runner.box.getSize().x/2, runner.box.getSize().y/2));
        window.draw(runner.box);
        runner.box.setPosition(runner.box.getPosition() + sf::Vector2f(runner.box.getSize().x/2, runner.box.getSize().y/2));
    }
}


