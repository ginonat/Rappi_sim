#include  "../include/draw.h"


void drawArrows(sf::RenderWindow& window, Node* node) {
    const sf::Color lineColor = sf::Color::White; // color of the lines
    const float arrowSize = 10.f; // size of the arrowhead
    const float pi = 3.14159265358979323846f; // the value of pi
    
    // Load a font
    sf::Font font;
    if (!font.loadFromFile("include/arial.ttf")) {
        // handle error if the font file could not be loaded
    }
    
    // Iterate through the node's neighbors and draw lines to them
    for (std::vector<Node*>::size_type i = 0; i < node->neighbors.size(); i++) {


        // Get the current neighbor
        const auto& neighbor = node->neighbors[i];
        
        // calculate the direction of the line
        sf::Vector2f dir = neighbor->position - node->position;
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir /= length;
        
        // calculate the midpoint between the node and its neighbor
        sf::Vector2f midpoint = node->position + dir * length / 2.f;
        
        // draw the line
        sf::Vertex line[] = {
            sf::Vertex(node->position, lineColor),
            sf::Vertex(neighbor->position, lineColor)
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);

        // calculate the angle of the arrowhead
        float angle = std::atan2(dir.y, dir.x);
        sf::Transform transform;
        transform.rotate(angle * 180.f / pi);
        
        // draw the arrowhead
        sf::ConvexShape arrow;
        arrow.setPointCount(3);
        arrow.setPoint(0, sf::Vector2f(0.f, -arrowSize / 2.f));
        arrow.setPoint(1, sf::Vector2f(-arrowSize, arrowSize / 2.f));
        arrow.setPoint(2, sf::Vector2f(0.f, arrowSize / 4.f));
        arrow.setFillColor(lineColor);
        arrow.setOutlineThickness(0.f);
        arrow.setPosition(neighbor->position);
        arrow.setOrigin(sf::Vector2f(arrowSize / 2.f, 0.f));
        arrow.setRotation(angle * 180.f / pi);
        window.draw(arrow);

        // create a text object to display the index
        sf::Text text(std::to_string(i), sf::Font(), 16);
        // draw the neighbor index in the midpoint
        text.setFont(font);
        text.setString(std::to_string(i));
        text.setCharacterSize(14);
        text.setFillColor(lineColor);
        text.setOutlineColor(lineColor);
        text.setOutlineThickness(0.01f);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width / 2.f, textBounds.height / 2.f);
        text.setPosition(midpoint);
        window.draw(text);
    }
}

