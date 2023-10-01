#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include<array> 
#include <vector>
#include "Quadtree.h"
#include "main.h"


using namespace std;
using namespace sf;

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

Quad::Quad(const BoundingBox& bounds_)
    : bounds(bounds_) {
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


    // If number of particles exceeds a threshold, split the quad and classify particles to children
    if (totalMass > 1) { // Threshold of 1 for now
        if (NW == nullptr) {  // To ensure that we don't create children again
            CreateChildren();
        }

        centerOfMass = Vector2f(0, 0);
        for (const auto& particle : particles) {
            centerOfMass += particle->position;
        }
        centerOfMass /= totalMass;

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
    else if (totalMass == 1) {
        centerOfMass = particles[0]->position;
    }
    else {
        centerOfMass = Vector2f(0, 0);
    }



}

void Quad::CreateChildren() {
    float halfWidth = bounds.width / 2;
    float halfHeight = bounds.height / 2;

    NW = new Quad(BoundingBox(bounds.x, bounds.y, halfWidth, halfHeight));
    NE = new Quad(BoundingBox(bounds.x + halfWidth, bounds.y, halfWidth, halfHeight));
    SW = new Quad(BoundingBox(bounds.x, bounds.y + halfHeight, halfWidth, halfHeight));
    SE = new Quad(BoundingBox(bounds.x + halfWidth, bounds.y + halfHeight, halfWidth, halfHeight));
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

    if (this->isLeaf()) {
        for (const auto& particle : particles) {
            if (particle == obj) continue;

            // Correctly compute the shortest direction considering the wrap-around
            Vector2f direction = particle->position - obj->position;
            direction.x = std::fmod(direction.x + 1000 / 2, 1000) - 1000 / 2;  // for wrap-around along x
            direction.y = std::fmod(direction.y + 1000 / 2, 1000) - 1000 / 2;  // for wrap-around along y

            float sqDist = squaredMagnitude(direction);

            float f = GRAVITY_CONSTANT * (1 / (sqDist + softening * softening));
            float magnitude = std::sqrt(sqDist);
            direction /= magnitude;
            force += direction * f;
        }
    }
    else {
        float d = wrappedDistance2D(obj->position, this->centerOfMass, 1000, 1000);
        if (this->bounds.width / d < theta) {
            Vector2f direction = this->centerOfMass - obj->position;
            direction.x = std::fmod(direction.x + 1000 / 2, 1000) - 1000 / 2;
            direction.y = std::fmod(direction.y + 1000 / 2, 1000) - 1000 / 2;

            float sqDist = squaredMagnitude(direction);

            float f = GRAVITY_CONSTANT * this->totalMass * (1 / (sqDist + softening * softening));
            float magnitude = std::sqrt(sqDist);
            direction /= magnitude;
            force = direction * f;
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