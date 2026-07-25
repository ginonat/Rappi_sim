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

namespace {
sf::Vector2f quadraticBezier(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, float t) {
    float u = 1.f - t;
    return u * u * p0 + 2.f * u * t * p1 + t * t * p2;
}
}

void drawRequests(sf::RenderWindow& window, const std::vector<Request>& requests, std::vector<sf::RectangleShape>& packages, float time) {
    for (auto& package : packages) {
        window.draw(package);
    }

    const int dotCount = 24;
    const float curvature = 0.18f;
    const float flowSpeed = 80.f; // pixels travelled per second, same for every request regardless of distance

    for (std::size_t i = 0; i < requests.size(); ++i) {
        const Request& request = requests[i];
        if (request.satisfied) {
            continue;
        }
        sf::Vector2f p0 = request.holder->position;
        sf::Vector2f p2 = request.destination->position;
        sf::Vector2f mid = (p0 + p2) / 2.f;
        sf::Vector2f dir = p2 - p0;
        float distance = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        sf::Vector2f perp(-dir.y, dir.x);
        if (distance > 0.f) {
            perp /= distance;
        }
        sf::Vector2f p1 = mid + perp * (curvature * distance);

        // Dim dotted curve showing the full path
        for (int d = 0; d <= dotCount; ++d) {
            float t = static_cast<float>(d) / dotCount;
            sf::Vector2f pos = quadraticBezier(p0, p1, p2, t);
            sf::CircleShape dot(1.5f);
            dot.setFillColor(sf::Color(255, 255, 255, 70));
            dot.setPosition(pos - sf::Vector2f(1.5f, 1.5f));
            window.draw(dot);
        }

        // One brighter dot per request, flowing from the shop towards the destination at a
        // constant real speed (time since the request was placed, not a path-length fraction,
        // so short and long routes move at the same pace instead of long ones looking faster).
        if (distance > 0.f) {
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


