
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class SolarObject; // Forward declaration

struct BoundingBox {
    float x, y, width, height;
    BoundingBox(float x_ = 0, float y_ = 0, float width_ = 1, float height_ = 1);
    bool contains(const sf::Vector2f& point) const;
    sf::VertexArray draw() const;
};

class Quadtree {
private:
    class Node {
    public:
        BoundingBox bounds;       // Boundary of the node
        std::vector<SolarObject*> particles; // Particles within this node
        std::unique_ptr<Node> NW; // Pointers to child nodes
        std::unique_ptr<Node> NE;
        std::unique_ptr<Node> SW;
        std::unique_ptr<Node> SE;
        sf::Vector2f centerOfMass;    // Center of mass of particles within node
        float totalMass;          // Total mass of particles within node
        int depth;

        Node(const BoundingBox& bounds_, const int depth_);

        void insert(SolarObject* particle);
        void subdivide();
        void placeInChild(SolarObject* particle);
        void clear();
        void updateCenterOfMassAndTotalMass();
        sf::Vector2f computeForce(const SolarObject& obj, float theta, float softening) const;

        // Debug

        void draw(sf::RenderWindow& window) const;
    };

    std::unique_ptr<Node> root; // Root node of the quadtree

public:
    Quadtree(const BoundingBox& bounds);

    void insert(SolarObject* particle);
    void draw(sf::RenderWindow& window) const;
    void clear();
    sf::Vector2f computeForce(const SolarObject& obj, float theta, float softening) const;
};