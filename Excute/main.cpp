#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

bool checkCollision(const sf::CircleShape& ball1, const sf::CircleShape& ball2) {
    float distance = std::hypot(ball1.getPosition().x - ball2.getPosition().x,
                                ball1.getPosition().y - ball2.getPosition().y);
    return distance < (ball1.getRadius() + ball2.getRadius());
}

void handleCollision(sf::CircleShape& ball1, sf::Vector2f& velocity1, sf::CircleShape& ball2, sf::Vector2f& velocity2) {
    sf::Vector2f collisionNormal(ball2.getPosition() - ball1.getPosition());
    float distance = std::sqrt(collisionNormal.x * collisionNormal.x + collisionNormal.y * collisionNormal.y);
    collisionNormal /= distance;

    sf::Vector2f relativeVelocity = velocity2 - velocity1;

    float velocityAlongNormal = relativeVelocity.x * collisionNormal.x + relativeVelocity.y * collisionNormal.y;

    if (velocityAlongNormal > 0)
        return;

    float restitution = 0.7f;

    float j = -(1 + restitution) * velocityAlongNormal;
    j /= 1 / ball1.getRadius() + 1 / ball2.getRadius();

    sf::Vector2f impulse = j * collisionNormal;
    velocity1 -= impulse / ball1.getRadius();
    velocity2 += impulse / ball2.getRadius();

    const float percent = 0.2f;
    const float slop = 0.01f;
    sf::Vector2f correction = std::max(distance - (ball1.getRadius() + ball2.getRadius()), 0.0f) / (1 / ball1.getRadius() + 1 / ball2.getRadius()) * percent * collisionNormal;
    ball1.setPosition(ball1.getPosition() - correction / ball1.getRadius());
    ball2.setPosition(ball2.getPosition() + correction / ball2.getRadius());
}

int main() {
    sf::RenderWindow window(sf::VideoMode(300, 300), "Dynamic Ball Simulation");
    window.setFramerateLimit(60);

    std::vector<sf::CircleShape> balls;
    std::vector<sf::Vector2f> velocities;

    sf::RectangleShape platform(sf::Vector2f(window.getSize().x, 20));
    platform.setPosition(0, window.getSize().y - platform.getSize().y);
    platform.setFillColor(sf::Color::Red);

    float gravity = 200.0f;
    float bounceFactor = -0.9f;
    float maxVelocity = 300.0f;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::CircleShape newBall(20.f);
                newBall.setFillColor(sf::Color::Green);
                newBall.setPosition(sf::Vector2f(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y));
                balls.push_back(newBall);
                velocities.push_back(sf::Vector2f(0.0f, 0.0f));
            }
        }

        float deltaTime = clock.restart().asSeconds() * 2.0f;

        for (size_t i = 0; i < balls.size(); ++i) {
            velocities[i].y += gravity * deltaTime;
            velocities[i].y = std::min(velocities[i].y, maxVelocity);
            velocities[i].x = std::clamp(velocities[i].x, -maxVelocity, maxVelocity);
            balls[i].move(velocities[i] * deltaTime);

            if (balls[i].getPosition().y + balls[i].getRadius() * 2 > platform.getPosition().y) {
                balls[i].setPosition(balls[i].getPosition().x, platform.getPosition().y - balls[i].getRadius() * 2);
                velocities[i].y *= bounceFactor;
            }

            for (size_t j = i + 1; j < balls.size(); ++j) {
                if (checkCollision(balls[i], balls[j])) {
                    handleCollision(balls[i], velocities[i], balls[j], velocities[j]);
                }
            }
        }

        window.clear();
        window.draw(platform);
        for (auto &ball : balls) {
            window.draw(ball);
        }
        window.display();
    }

    return 0;
}
