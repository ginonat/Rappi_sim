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
// Generic labeled slider: "label: value<suffix>" above a draggable track+handle. Used for
// the runner/simulation speed dials and every economy dial in the settings panel.
void drawSlider(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2f& position, float width,
                 const std::string& label, float minValue, float maxValue, float value,
                 int decimals = 2, const std::string& suffix = "");
void drawStartMenu(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& loadMapButton,
                    const std::vector<sf::FloatRect>& mapButtons, const std::vector<std::string>& mapLabels,
                    bool showMapList, const sf::Vector2i& mousePixel);
void drawPreviewRect(sf::RenderWindow& window, const sf::FloatRect& rect, sf::Color color);
void drawButton(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& bounds,
                 const std::string& label, bool hovered);
// Semi-opaque box with a titled header and a border - the shared backdrop for the
// settings and stats overlay panels.
void drawPanelBackground(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& bounds, const std::string& title);
// One or more time series drawn as connected lines inside `bounds`, auto-scaled to the
// data's own min/max, with a small color-swatch legend. Every series in `series` must be
// the same length; `series[i]` is drawn in `colors[i]` and labeled `labels[i]`.
void drawLineChart(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& bounds, const std::string& title,
                    const std::vector<std::vector<float>>& series, const std::vector<sf::Color>& colors,
                    const std::vector<std::string>& labels);

#endif // DRAW_H
