#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

extern const float GRAVITY_CONSTANT;

// Function Declarations
float squaredMagnitude(const sf::Vector2f& a);
float squaredDistance(const sf::Vector2f& a, const sf::Vector2f& b);
float wrappedDistance(float pos1, float pos2, float maxDimension);
float wrappedDistance2D(const sf::Vector2f& a, const sf::Vector2f& b, float screenWidth, float screenHeight);
float random_float(float min, float max);

// SolarObject Class
class SolarObject {
public:
    int mass;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;
    sf::CircleShape shape;

    void UpdateVelocity(const std::vector<SolarObject>& particles);
    void UpdatePosition();
    sf::CircleShape& Draw();
    SolarObject(int mass_ = 100,
        const sf::Vector2f& position_ = sf::Vector2f(0, 0),
        const sf::Vector2f& velocity_ = sf::Vector2f(0, 0),
        float radius_ = 1);
};