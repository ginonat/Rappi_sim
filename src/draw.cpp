#include  "../include/draw.h"
#include <string>
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

void drawRequests(sf::RenderWindow& window, std::vector<sf::RectangleShape>& packages, std::vector<sf::VertexArray>& arrows) {
    for (auto& package : packages) {
        window.draw(package);
    }
    for (auto& arrow : arrows) {
        window.draw(arrow);
    }
}

void drawRunners(sf::RenderWindow& window, std::vector<Runner>& runners) {
    for (auto& runner : runners) {
        // Draw the runner
        runner.box.setPosition(runner.box.getPosition() - sf::Vector2f(runner.box.getSize().x/2, runner.box.getSize().y/2));
        window.draw(runner.box);
        runner.box.setPosition(runner.box.getPosition() + sf::Vector2f(runner.box.getSize().x/2, runner.box.getSize().y/2));
    }
}

void drawSidebar(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& size,
                  bool edit_mode, std::size_t numRunners, std::size_t numShops, std::size_t numRequests) {
    sf::RectangleShape background(size);
    background.setPosition(position);
    background.setFillColor(sf::Color(15, 15, 15, 220));
    window.draw(background);

    std::string body =
        "Rappi_sim\n\n"
        "Controls\n"
        "E  edit mode\n"
        "S  mark shop (edit)\n"
        "G  save map (edit)\n"
        "L  load map (edit)\n"
        "+  spawn runner\n"
        "scroll  zoom\n\n"
        "Stats\n"
        "Mode: " + std::string(edit_mode ? "Edit" : "Running") + "\n"
        "Runners: " + std::to_string(numRunners) + "\n"
        "Shops: " + std::to_string(numShops) + "\n"
        "Requests: " + std::to_string(numRequests);

    sf::Text text(body, font, 14);
    text.setFillColor(sf::Color::White);
    text.setPosition(position + sf::Vector2f(10.f, 10.f));
    window.draw(text);
}


