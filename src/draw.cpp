#include  "../include/draw.h"
#include <iomanip>
#include <sstream>
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

namespace {
sf::Vector2f quadraticBezier(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, float t) {
    float u = 1.f - t;
    return u * u * p0 + 2.f * u * t * p1 + t * t * p2;
}
}

void drawRequests(sf::RenderWindow& window, const std::list<Request>& requests, float time) {
    const int dotCount = 24;
    const float curvature = 0.18f;
    const float flowSpeed = 80.f; // pixels travelled per second, same for every request regardless of distance

    for (const auto& request : requests) {
        if (request.satisfied) {
            continue;
        }

        // Once a runner has picked up the package, the path is drawn live from the
        // runner's current position instead of the shop's fixed one, and switches to blue.
        bool inTransit = request.designatedRunner != nullptr && request.designatedRunner->hasPackage;

        if (!inTransit) {
            // Package still waiting at the shop
            sf::RectangleShape package(sf::Vector2f(10, 10));
            package.setPosition(request.holder->position);
            window.draw(package);
        }

        sf::Vector2f p0 = inTransit ? request.designatedRunner->box.getPosition() : request.holder->position;
        sf::Vector2f p2 = request.destination->position;
        sf::Vector2f mid = (p0 + p2) / 2.f;
        sf::Vector2f dir = p2 - p0;
        float distance = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        sf::Vector2f perp(-dir.y, dir.x);
        if (distance > 0.f) {
            perp /= distance;
        }
        sf::Vector2f p1 = mid + perp * (curvature * distance);

        sf::Color dimColor = inTransit ? sf::Color(90, 170, 255, 90) : sf::Color(255, 255, 255, 70);

        // Dim dotted curve showing the full path
        for (int d = 0; d <= dotCount; ++d) {
            float t = static_cast<float>(d) / dotCount;
            sf::Vector2f pos = quadraticBezier(p0, p1, p2, t);
            sf::CircleShape dot(1.5f);
            dot.setFillColor(dimColor);
            dot.setPosition(pos - sf::Vector2f(1.5f, 1.5f));
            window.draw(dot);
        }

        // One brighter dot per pending (not yet picked up) request, flowing from the shop
        // towards the destination at a constant real speed (time since the request was
        // placed, not a path-length fraction, so short and long routes move at the same
        // pace instead of long ones looking faster). Once a runner is carrying the package,
        // the runner itself (drawn by drawRunners) is the moving marker, so no separate dot.
        if (!inTransit && distance > 0.f) {
            float elapsed = time - request.spawnTime;
            float t = std::fmod(elapsed * flowSpeed / distance, 1.f);
            sf::Vector2f pos = quadraticBezier(p0, p1, p2, t);
            sf::CircleShape flowDot(2.5f);
            flowDot.setFillColor(sf::Color(255, 220, 80, 220));
            flowDot.setPosition(pos - sf::Vector2f(2.5f, 2.5f));
            window.draw(flowDot);
        }
    }
}

void drawRunners(sf::RenderWindow& window, std::deque<Runner>& runners) {
    for (auto& runner : runners) {
        // Draw the runner
        runner.box.setFillColor(runner.activeRequest == nullptr ? sf::Color::Green : sf::Color::Blue);
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
        "scroll  zoom\n"
        "middle drag  pan\n\n"
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

void drawSpeedSlider(sf::RenderWindow& window, const sf::Font& font, const sf::Vector2f& position, float width,
                      float minValue, float maxValue, float value) {
    std::ostringstream label;
    label << "Speed: " << std::fixed << std::setprecision(2) << value << "x";
    sf::Text text(label.str(), font, 14);
    text.setFillColor(sf::Color::White);
    text.setPosition(position);
    window.draw(text);

    float trackY = position.y + 22.f;
    sf::RectangleShape track(sf::Vector2f(width, 4.f));
    track.setPosition(position.x, trackY);
    track.setFillColor(sf::Color(90, 90, 90));
    window.draw(track);

    float t = (value - minValue) / (maxValue - minValue);
    t = std::max(0.f, std::min(1.f, t));
    sf::CircleShape handle(6.f);
    handle.setFillColor(sf::Color(0, 200, 255));
    handle.setPosition(position.x + t * width - handle.getRadius(), trackY + 2.f - handle.getRadius());
    window.draw(handle);
}

namespace {
void drawButton(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& bounds,
                 const std::string& label, bool hovered) {
    sf::RectangleShape box(sf::Vector2f(bounds.width, bounds.height));
    box.setPosition(bounds.left, bounds.top);
    box.setFillColor(hovered ? sf::Color(60, 60, 60) : sf::Color(35, 35, 35));
    box.setOutlineColor(sf::Color(120, 120, 120));
    box.setOutlineThickness(1.f);
    window.draw(box);

    sf::Text text(label, font, 16);
    text.setFillColor(sf::Color::White);
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
    text.setPosition(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    window.draw(text);
}
}

void drawStartMenu(sf::RenderWindow& window, const sf::Font& font, const sf::FloatRect& loadMapButton,
                    const std::vector<sf::FloatRect>& mapButtons, const std::vector<std::string>& mapLabels,
                    bool showMapList, const sf::Vector2i& mousePixel) {
    sf::Vector2f mouse(static_cast<float>(mousePixel.x), static_cast<float>(mousePixel.y));

    sf::Text title("Rappi_sim", font, 40);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
    title.setPosition(window.getSize().x / 2.f, loadMapButton.top - 80.f);
    title.setFillColor(sf::Color::White);
    window.draw(title);

    sf::Text subtitle("a delivery-courier simulation", font, 14);
    sf::FloatRect subtitleBounds = subtitle.getLocalBounds();
    subtitle.setOrigin(subtitleBounds.left + subtitleBounds.width / 2.f, subtitleBounds.top + subtitleBounds.height / 2.f);
    subtitle.setPosition(window.getSize().x / 2.f, loadMapButton.top - 40.f);
    subtitle.setFillColor(sf::Color(160, 160, 160));
    window.draw(subtitle);

    drawButton(window, font, loadMapButton, "Load Map", loadMapButton.contains(mouse));

    if (showMapList) {
        if (mapLabels.empty()) {
            sf::Text empty("No maps found in maps/", font, 14);
            empty.setFillColor(sf::Color(160, 160, 160));
            empty.setPosition(loadMapButton.left, loadMapButton.top + loadMapButton.height + 12.f);
            window.draw(empty);
        } else {
            for (std::size_t i = 0; i < mapButtons.size() && i < mapLabels.size(); ++i) {
                drawButton(window, font, mapButtons[i], mapLabels[i], mapButtons[i].contains(mouse));
            }
        }
    }
}


