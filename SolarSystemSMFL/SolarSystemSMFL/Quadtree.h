
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "main.h"

struct BoundingBox {
    float x, y, width, height;
    BoundingBox(float x_ = 0, float y_ = 0, float width_ = 1, float height_ = 1);
    bool contains(const sf::Vector2f& point) const;
    sf::VertexArray draw() const;
};

class Quad {
public:
    BoundingBox bounds;
    Quad* NW;
    Quad* NE;
    Quad* SW;
    Quad* SE;
    std::vector<SolarObject*> particles;

    Quad(const BoundingBox& bounds_);
    ~Quad();
    bool isLeaf() const;
    bool classifyParticle(SolarObject* p);
    void BatchParticles();
    void CreateChildren();
    void Draw(sf::RenderWindow& window) const;
    void Reset();
};