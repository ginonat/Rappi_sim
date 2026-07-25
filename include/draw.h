#ifndef DRAW_H
#define DRAW_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "struct.h"

void drawCity(sf::RenderWindow& window, const std::vector<Node>& nodes, sf::CircleShape nodeCircle, const sf::VertexArray& streets, sf::CircleShape& shopCircle);
void drawRunners(sf::RenderWindow& window, std::vector<Runner>& runners);
void drawEditMode(sf::RenderWindow& window, const bool edit_mode, const sf::CircleShape& nodeCircle, const sf::CircleShape& shopCircle);
void drawRequests(sf::RenderWindow& window, std::vector<sf::RectangleShape>& packages, std::vector<sf::VertexArray>& arrows);
void drawSidebar(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& size,
                  bool edit_mode, std::size_t numRunners, std::size_t numShops, std::size_t numRequests);

#endif // DRAW_H
