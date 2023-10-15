#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include<array> 
#include <random>
#include <vector>
#include "Quadtree.h"
#include "main.h"

using namespace std;
using namespace sf;


// Implementation for UpdateVelocity
void SolarObject::UpdateVelocity(Quadtree& tree) {

    Vector2f force = tree.computeForce(*this);

//    cout << force.x << " " << force.y << endl;

    velocity += force;
}

// Implementation for UpdatePosition
void SolarObject::UpdatePosition() {
    position += velocity;

    // Wrap around logic for X-axis
    if (position.x < 0) position.x += 1000;
    else if (position.x > 1000) position.x -= 1000;

    // Wrap around logic for Y-axis
    if (position.y < 0) position.y += 1000;
    else if (position.y > 1000) position.y -= 1000;
}

// Implementation for the constructor
SolarObject::SolarObject(const sf::Vector2f& position_, const sf::Vector2f& velocity_)
    : position(position_), velocity(velocity_) {

}

float random_float(float min, float max) {
    static std::uniform_real_distribution<float> distribution(min, max);
    static std::mt19937 generator;
    return distribution(generator);
}



int main()
{
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "N-Body simulation");

    // Load Font
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) { // replace 'path_to_font.ttf' with the path to your font file
        std::cerr << "Failed to load font." << std::endl;
        return 1;
    }

    // Declare a text shape
    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(24); 
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition(10, 10); 

    sf::Clock clock;  // Clock to measure time since the last update
    const sf::Time TimePerFrame = sf::seconds(1.0f / 30.0f);  // 30 updates per second
    sf::Time timeSinceLastUpdate = sf::Time::Zero;


    vector<SolarObject> particles(1000);





    float timeElapsed = 0; // Total elapsed time
    int frameCount = 0;    // Number of frames elapsed

    for (int i = 0; i < particles.size(); i++)
    {
        particles[i] = SolarObject(Vector2f(random_float(50.0f, 950.0f), random_float(50.0f, 950.0f)), Vector2f(0, 0));
    }

    Quadtree tree(BoundingBox(0, 0, 1000, 1000));

    sf::VertexArray particlesVertexArray(sf::Points, particles.size());

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;

        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;

            window.clear();

            tree.clear();
            for (auto& particle : particles) {
                tree.insert(&particle);
            }

            for (auto& particle : particles) {
                particle.UpdateVelocity(tree);
            }

            int index = 0;
            for (auto& particle : particles) {
                particle.UpdatePosition();
                particlesVertexArray[index].position = particle.position;
                particlesVertexArray[index].color = sf::Color(255, 255, 255,60);
                ++index;
            }
            window.draw(particlesVertexArray);

          //  tree.draw(window);

            float deltaTime = clock.restart().asSeconds();
            timeElapsed += deltaTime;

            window.display();
        }

    }

    return 0;
}