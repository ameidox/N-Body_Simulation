#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include<array> 
#include <vector>
#include "Quadtree.h"
#include "main.h"


using namespace std;
using namespace sf;

const int MAX_DEPTH = 25;

BoundingBox::BoundingBox(float x_, float y_, float width_, float height_)
    : x(x_), y(y_), width(width_), height(height_) {}

bool BoundingBox::contains(const sf::Vector2f& point) const {
    return (point.x >= x) &&
        (point.x <= x + width) &&
        (point.y >= y) &&
        (point.y <= y + height);
}


sf::VertexArray BoundingBox::draw() const {
    sf::VertexArray lines(sf::LineStrip, 5); // 5 points to close the loop
    lines[0].position = sf::Vector2f(x, y);
    lines[1].position = sf::Vector2f(x + width, y);
    lines[2].position = sf::Vector2f(x + width, y + height);
    lines[3].position = sf::Vector2f(x, y + height);
    lines[4].position = sf::Vector2f(x, y); // back to the starting point to close the outline

    for (int i = 0; i < 5; ++i) {
        lines[i].color = sf::Color::White;
    }

    return lines;
}

Quad::Quad(const BoundingBox& bounds_ = BoundingBox(), const int depth_=0)
    : bounds(bounds_), depth(depth_)
{
    NW = nullptr;
    NE = nullptr;
    SW = nullptr;
    SE = nullptr;
}

Quad::~Quad() {

    if (NW) {
        delete NW;
        NW = nullptr;
    }
    if (NE) {
        delete NE;
        NE = nullptr;
    }
    if (SW) {
        delete SW;
        SW = nullptr;
    }
    if (SE) {
        delete SE;
        SE = nullptr;
    }
}

bool Quad::isLeaf() const {
    return particles.size() <= 1;
}

bool Quad::classifyParticle(SolarObject* p) {
    if (!bounds.contains(p->position)) {
        return false;  // Particle doesn't belong here
    }
    particles.push_back(p);
    return true;
}

void Quad::BatchParticles() {

    totalMass = (int)particles.size();

    centerOfMass = Vector2f(0, 0);
    for (const auto& particle : particles) {
        centerOfMass += particle->position;
    }
    centerOfMass /= totalMass;

    // If number of particles exceeds a threshold, split the quad and classify particles to children
    if (totalMass > 1 && depth <= MAX_DEPTH) { // Threshold of 1 for now
        if (NW == nullptr) {  // To ensure that we don't create children again
            CreateChildren();
        }

        for (const auto& particle : particles) {
            NW->classifyParticle(particle) ||
            NE->classifyParticle(particle) ||
            SW->classifyParticle(particle) ||
            SE->classifyParticle(particle);
        }
        // Recursively batch particles for children
        NW->BatchParticles();
        NE->BatchParticles();
        SW->BatchParticles();
        SE->BatchParticles();
    }

}

void Quad::CreateChildren() {
    float halfWidth = bounds.width / 2;
    float halfHeight = bounds.height / 2;

    NW = new Quad(BoundingBox(bounds.x, bounds.y, halfWidth, halfHeight), depth+1);
    NE = new Quad(BoundingBox(bounds.x + halfWidth, bounds.y, halfWidth, halfHeight), depth + 1);
    SW = new Quad(BoundingBox(bounds.x, bounds.y + halfHeight, halfWidth, halfHeight), depth + 1);
    SE = new Quad(BoundingBox(bounds.x + halfWidth, bounds.y + halfHeight, halfWidth, halfHeight) ,depth + 1);
}

void Quad::Draw(sf::RenderWindow& window) const {

    // Draw the bounding box for this Quad
    window.draw(bounds.draw());

    // Recursively draw children (if they exist)
    if (NW) NW->Draw(window);
    if (NE) NE->Draw(window);
    if (SW) SW->Draw(window);
    if (SE) SE->Draw(window);
}

Vector2f Quad::ComputeForce(SolarObject* obj, float theta, float softening) {
    Vector2f force(0, 0);

    const float HALF_DIMENSION = 500.0f;
    const float DOMAIN_SIZE = 1000.0f;

    if (this->isLeaf() || this->depth == MAX_DEPTH) {
        for (const auto& particle : particles) {
            if (particle == obj) continue;

            Vector2f direction = particle->position - obj->position;
            if (direction.x < -HALF_DIMENSION) direction.x += DOMAIN_SIZE;
            else if (direction.x > HALF_DIMENSION) direction.x -= DOMAIN_SIZE;
            if (direction.y < -HALF_DIMENSION) direction.y += DOMAIN_SIZE;
            else if (direction.y > HALF_DIMENSION) direction.y -= DOMAIN_SIZE;

            float sqDist = squaredMagnitude(direction);
            float magnitude = std::sqrt(sqDist);
            float f_scalar = GRAVITY_CONSTANT * (1 / (sqDist + softening * softening));
            force += (direction / magnitude) * f_scalar;
        }
    }
    else {
        float d = wrappedDistance2D(obj->position, this->centerOfMass);
        if (this->bounds.width / d < theta) {
            Vector2f direction = this->centerOfMass - obj->position;
            if (direction.x < -HALF_DIMENSION) direction.x += DOMAIN_SIZE;
            else if (direction.x > HALF_DIMENSION) direction.x -= DOMAIN_SIZE;
            if (direction.y < -HALF_DIMENSION) direction.y += DOMAIN_SIZE;
            else if (direction.y > HALF_DIMENSION) direction.y -= DOMAIN_SIZE;

            float sqDist = squaredMagnitude(direction);
            float magnitude = std::sqrt(sqDist);
            float f_scalar = GRAVITY_CONSTANT * this->totalMass * (1 / (sqDist + softening * softening));
            force += (direction / magnitude) * f_scalar;
        }
        else {
            if (NW) force += NW->ComputeForce(obj, theta, softening);
            if (NE) force += NE->ComputeForce(obj, theta, softening);
            if (SW) force += SW->ComputeForce(obj, theta, softening);
            if (SE) force += SE->ComputeForce(obj, theta, softening);
        }
    }
    return force;
}




void Quad::Reset() {

    if (depth != 0) { return; }

    if (NW) {
        delete NW;
        NW = nullptr;
    }
    if (NE) {
        delete NE;
        NE = nullptr;
    }
    if (SW) {
        delete SW;
        SW = nullptr;
    }
    if (SE) {
        delete SE;
        SE = nullptr;
    }
}