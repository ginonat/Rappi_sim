#ifndef DRAW_H
#define DRAW_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "struct.h"

void drawCity(sf::RenderWindow& window, const std::vector<Node>& nodes, sf::CircleShape nodeCircle, const sf::VertexArray& streets, sf::CircleShape& shopCircle);
void drawRunners(sf::RenderWindow& window, std::vector<Runner>& runners);
void drawEditMode(sf::RenderWindow& window, const bool edit_mode, const sf::CircleShape& nodeCircle, const sf::CircleShape& shopCircle);

#endif // DRAW_H
