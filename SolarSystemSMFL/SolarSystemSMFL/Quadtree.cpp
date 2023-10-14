#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include<array> 
#include <vector>
#include <stack>
#include "Quadtree.h"
#include "main.h"


using namespace std;
using namespace sf;

const int MAX_DEPTH = 105;
const int THRESHOLD = 5;

#pragma region BoundingBox
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

#pragma endregion

#pragma region Quadtree

Quadtree::Quadtree(const BoundingBox& bounds)
{
    root = std::make_unique<Node>(bounds, 0);
}
 
void Quadtree::insert(SolarObject* particle) {
    root->insert(particle);
}

void Quadtree::draw(sf::RenderWindow& window) const {
    if (root) {
        root->draw(window);
    }
}

sf::Vector2f Quadtree::computeForce(const SolarObject& obj, float theta, float softening) const {
    return root->computeForce(obj, theta, softening);
}

void Quadtree::clear() {
    root->clear();
}

#pragma endregion

#pragma region Node

Quadtree::Node::Node(const BoundingBox& bounds_, const int depth_)
 : bounds(bounds_), depth(depth_)
{
    NW = nullptr;
    NE = nullptr;
    SE = nullptr;
    SW = nullptr;
}

void Quadtree::Node::insert(SolarObject* particle)
{
    if (!bounds.contains(particle->position)) return;

    // Check if node is leaf (number of particles inside of it is lesser than the threshold)
    if (NW == nullptr) {
        if (particles.size() < THRESHOLD) {
            // Add partical to list
            particles.push_back(particle);
        }
        else 
        {
            subdivide();
            // If there are more particals than the threshold subdivide the node
            for (SolarObject* existingParticle : particles) {
                // Re-insert existing particles into the correct child node
                placeInChild(existingParticle);
            }
            // Clear the particles vector of the current node
            particles.clear();
            // Insert the new particle into the correct child node
            placeInChild(particle);
        }
    }
    else 
    {
        // If it's not a leaf assign the partical to one of its children nodes
        placeInChild(particle);
    }

    updateCenterOfMassAndTotalMass();
}

void Quadtree::Node::updateCenterOfMassAndTotalMass() {
    Vector2f newCenterOfMass(0, 0);
    float newTotalMass = 0;

    // Sum up the contributions from the node's particles
    for (const auto& particle : this->particles) {
        newCenterOfMass += particle->position * particle->mass;
        newTotalMass += particle->mass;
    }

    // If the node has children, sum up their contributions too
    if (this->NW) {
        newCenterOfMass += this->NW->centerOfMass * this->NW->totalMass;
        newTotalMass += this->NW->totalMass;

        newCenterOfMass += this->NE->centerOfMass * this->NE->totalMass;
        newTotalMass += this->NE->totalMass;

        newCenterOfMass += this->SW->centerOfMass * this->SW->totalMass;
        newTotalMass += this->SW->totalMass;

        newCenterOfMass += this->SE->centerOfMass * this->SE->totalMass;
        newTotalMass += this->SE->totalMass;
    }


    // Divide to get the weighted average
    if (newTotalMass != 0) {
        newCenterOfMass /= newTotalMass;
    }

    this->centerOfMass = newCenterOfMass;
    this->totalMass = newTotalMass;
}

void Quadtree::Node::subdivide() {
    float halfWidth = bounds.width / 2;
    float halfHeight = bounds.height / 2;

    NW = std::make_unique<Node>(BoundingBox(bounds.x, bounds.y, halfWidth, halfHeight), depth + 1);
    NE = std::make_unique<Node>(BoundingBox(bounds.x + halfWidth, bounds.y, halfWidth, halfHeight), depth + 1);
    SW = std::make_unique<Node>(BoundingBox(bounds.x, bounds.y + halfHeight, halfWidth, halfHeight), depth + 1);
    SE = std::make_unique<Node>(BoundingBox(bounds.x + halfWidth, bounds.y + halfHeight, halfWidth, halfHeight), depth+1);
}

void Quadtree::Node::placeInChild(SolarObject* particle) {
    if (NW->bounds.contains(particle->position)) NW->insert(particle);
    else if (NE->bounds.contains(particle->position)) NE->insert(particle);
    else if (SW->bounds.contains(particle->position)) SW->insert(particle);
    else if (SE->bounds.contains(particle->position)) SE->insert(particle);
}

void Quadtree::Node::draw(sf::RenderWindow& window) const {
    // Draw the bounding box for this Node
   window.draw(bounds.draw());

    CircleShape circle;
    circle.setPosition(centerOfMass);
    circle.setRadius(totalMass / 2.0f);
    circle.setFillColor(Color(255, 0, 255, 255));
   // window.draw(circle);
    // Recursively draw children if they exist
    if (NW) NW->draw(window);
    if (NE) NE->draw(window);
    if (SW) SW->draw(window);
    if (SE) SE->draw(window);
}

void Quadtree::Node::clear() {
    NW.reset();
    NE.reset();
    SW.reset();
    SE.reset();

    particles.clear();
}

void wrapDirection(Vector2f& direction, float halfDim, float domainSize) {
    if (direction.x < -halfDim) direction.x += domainSize;
    else if (direction.x > halfDim) direction.x -= domainSize;
    if (direction.y < -halfDim) direction.y += domainSize;
    else if (direction.y > halfDim) direction.y -= domainSize;
}

Vector2f Quadtree::Node::computeForce(const SolarObject& obj, float theta, float softening) const {
    Vector2f force(0, 0);
    const float HALF_DIMENSION = 500.0f;
    const float DOMAIN_SIZE = 1000.0f;

    if (totalMass < THRESHOLD) {
        for (const auto& particle : particles) {
            if (particle == &obj) continue;

            Vector2f direction = particle->position - obj.position;
            wrapDirection(direction, HALF_DIMENSION, DOMAIN_SIZE);

            float sqDist = squaredMagnitude(direction);
            float magnitude = std::sqrt(sqDist);
            float f_scalar = GRAVITY_CONSTANT * (1 / (sqDist + softening * softening));

            force += (direction / magnitude) * f_scalar;
        }
    }
    else {
        float d = wrappedDistance2D(obj.position, this->centerOfMass);
        if (this->bounds.width / d < theta) {
            Vector2f direction = this->centerOfMass - obj.position;
            wrapDirection(direction, HALF_DIMENSION, DOMAIN_SIZE);

            float sqDist = squaredMagnitude(direction);
            float magnitude = std::sqrt(sqDist);
            float f_scalar = GRAVITY_CONSTANT * this->totalMass * (1 / (sqDist + softening * softening));
            force += (direction / magnitude) * f_scalar;
        }
        else {
            // Sum forces from all children
            Vector2f forcesFromChildren[4] = {
                NW ? NW->computeForce(obj, theta, softening) : Vector2f(0, 0),
                NE ? NE->computeForce(obj, theta, softening) : Vector2f(0, 0),
                SW ? SW->computeForce(obj, theta, softening) : Vector2f(0, 0),
                SE ? SE->computeForce(obj, theta, softening) : Vector2f(0, 0)
            };

            for (int i = 0; i < 4; i++) {
                force += forcesFromChildren[i];
            }
        }
    }

    return force;
}


#pragma endregion



