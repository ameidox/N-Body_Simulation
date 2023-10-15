#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Quadtree.h"

extern const float GRAVITY_CONSTANT;

float random_float(float min, float max);

// SolarObject Class
class SolarObject {
public:

    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;

    void UpdateVelocity(Quadtree& tree);
    void UpdatePosition();
    SolarObject(
        const sf::Vector2f& position_ = sf::Vector2f(0, 0),
        const sf::Vector2f& velocity_ = sf::Vector2f(0, 0));

};