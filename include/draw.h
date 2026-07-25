#ifndef DRAW_H
#define DRAW_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <deque>
#include <list>
#include <string>
#include <vector>
#include "struct.h"

void drawCity(sf::RenderWindow& window, const std::list<Node>& nodes, sf::CircleShape nodeCircle, const sf::VertexArray& streets, sf::CircleShape& shopCircle);
void drawRunners(sf::RenderWindow& window, std::deque<Runner>& runners);
void drawEditMode(sf::RenderWindow& window, const bool edit_mode, const sf::CircleShape& nodeCircle, const sf::CircleShape& shopCircle);
void drawRequests(sf::RenderWindow& window, const std::list<Request>& requests, float time);
void drawSidebar(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2f& position, const sf::Vector2f& size,
                  bool edit_mode, std::size_t numRunners, std::size_t numShops, std::size_t numRequests,
                  const std::string& currentTierLabel, bool gridSnapEnabled, int gridRows, int gridCols, bool gridToolArmed);
void drawSpeedSlider(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2f& position, float width,
                      float minValue, float maxValue, float value);
void drawStartMenu(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& loadMapButton,
                    const std::vector<sf::FloatRect>& mapButtons, const std::vector<std::string>& mapLabels,
                    bool showMapList, const sf::Vector2i& mousePixel);
void drawPreviewRect(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color);

#endif // DRAW_H
