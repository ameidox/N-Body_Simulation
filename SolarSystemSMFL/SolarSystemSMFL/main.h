#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Quadtree.h"

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
    float mass;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;

    void UpdateVelocity(Quad& rootNode);
    void UpdatePosition();
    SolarObject(int mass_,
        const sf::Vector2f& position_ = sf::Vector2f(0, 0),
        const sf::Vector2f& velocity_ = sf::Vector2f(0, 0));

};