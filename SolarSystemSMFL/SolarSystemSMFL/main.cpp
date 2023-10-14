#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include<array> 
#include <random>
#include <vector>
#include <thread>
#include "Quadtree.h"
#include "main.h"

using namespace std;
using namespace sf;

const float GRAVITY_CONSTANT = 1.8f;
const int NUM_THREADS = 12;


// Return squared magnitude
float squaredMagnitude(const Vector2f& a) {
    return a.x * a.x + a.y * a.y;
}
// Return squared distance
float squaredDistance(const Vector2f& a, const Vector2f& b) {
    Vector2f diff = a - b;
    return squaredMagnitude(diff);
}

float wrappedDistance(float pos1, float pos2, float maxDimension) {
    float directDist = std::abs(pos1 - pos2);

    // If direct distance is already less than half of the dimension, return it
    if (directDist <= maxDimension * 0.5f) {
        return directDist;
    }

    // Otherwise, return the wrap-around distance
    return maxDimension - directDist;
}

float wrappedDistance2D(const Vector2f& a, const Vector2f& b) {
    const float DOMAIN_SIZE = 1000.0f;
    const float HALF_DIMENSION = 500.0f;

    auto wrappedDistanceComponent = [DOMAIN_SIZE, HALF_DIMENSION](float comp1, float comp2) -> float {
        float dist = comp1 - comp2;
        if (dist < -HALF_DIMENSION) dist += DOMAIN_SIZE;
        else if (dist > HALF_DIMENSION) dist -= DOMAIN_SIZE;
        return dist;
    };

    float xDist = wrappedDistanceComponent(a.x, b.x);
    float yDist = wrappedDistanceComponent(a.y, b.y);

    return std::sqrt(xDist * xDist + yDist * yDist);
}



// Implementation for UpdateVelocity
void SolarObject::UpdateVelocity(Quad& rootNode) {
    const float softening = 3.5f;
    const float theta = 0.65f;

    Vector2f force = rootNode.ComputeForce(this, theta, softening);

    Vector2f velocityChange = force / this->mass;
    velocity += velocityChange;
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
SolarObject::SolarObject(int mass_=1, const sf::Vector2f& position_, const sf::Vector2f& velocity_)
    : mass(mass_), position(position_), velocity(velocity_) {

}

float random_float(float min, float max) {
    static std::uniform_real_distribution<float> distribution(min, max);
    static std::mt19937 generator;
    return distribution(generator);
}

void UpdateVelocitiesThreaded(int start, int end, std::vector<SolarObject>& particles, Quad& rootNode) {
    for (int i = start; i < end; i++) {
        particles[i].UpdateVelocity(rootNode);
    }
}

void UpdatePositionsThreaded(int start, int end, std::vector<SolarObject>& particles) {
    for (int i = start; i < end; i++) {
        particles[i].UpdatePosition();
    }
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

    const float center_x = 1000/2;
    const float center_y = 1000/2;
    const float initial_radius = 30;         // Start further from the center
    const float spiral_arm_separation = 26;  // Increase distance between spiral arms
    const float angle_increment = 0.0002;     // Slightly smaller increment to space out particles more
    const float random_radius_max = 10;      // Increase randomness in the radius
    const float random_angle_max = 0.5; 

    // This constant can be adjusted to change the rotation speed of the particles
    const float rotation_speed_factor =0.2;

    float current_angle = 0;

    float timeElapsed = 0; // Total elapsed time
    int frameCount = 0;    // Number of frames elapsed

    for (int i = 0; i < particles.size(); i++)
    {
        float random_radius = random_float(-random_radius_max, random_radius_max);
        float random_angle = random_float(-random_angle_max, random_angle_max);

        float r = initial_radius + spiral_arm_separation * current_angle + random_radius;
        float theta = current_angle + random_angle;

        float x = center_x + r * cos(theta);
        float y = center_y + r * sin(theta);

        // Compute the tangential velocity for rotation
        Vector2f tangential_velocity(-sin(theta), cos(theta));
        tangential_velocity *= sqrt(r) * rotation_speed_factor;  // Using sqrt(r) ensures outer particles rotate slower

        particles[i] = SolarObject(1, Vector2f(x, y), tangential_velocity);

        current_angle += angle_increment;
    }

    Quad rootQuad(BoundingBox(0, 0, 1000, 1000), 0);
    for (auto& particle : particles) {
        rootQuad.particles.push_back(&particle);
    }
    rootQuad.BatchParticles();

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
            rootQuad.Reset();
            rootQuad.BatchParticles();

            for (auto& particle : particles) {
                particle.UpdateVelocity(rootQuad);
            }

            // Multithreading for updating positions
            std::vector<std::thread> threads;
            size_t chunkSize = particles.size() / NUM_THREADS;

            for (int i = 0; i < NUM_THREADS; ++i) {
                threads.emplace_back([i, chunkSize, &particles]() {
                    for (size_t j = i * chunkSize; j < (i + 1) * chunkSize; ++j) {
                        particles[j].UpdatePosition();
                    }
                    });
            }

            // Join the threads after their task
            for (auto& thread : threads) {
                thread.join();
            }

            // Sequentially update the vertex array after all threads have completed
            for (size_t i = 0; i < particles.size(); ++i) {
                particlesVertexArray[i].position = particles[i].position;
                particlesVertexArray[i].color = sf::Color(255, 255, 255, 70);
            }

            window.draw(particlesVertexArray);

          //  rootQuad.Draw(window);

            float deltaTime = clock.restart().asSeconds();
            timeElapsed += deltaTime;
            frameCount++;

            if (timeElapsed >= 1.0f) {  // update FPS once a second
                fpsText.setString("FPS: " + std::to_string(frameCount));
                frameCount = 0;
                timeElapsed -= 1.0f;
            }

            window.draw(fpsText);

            window.display();
        }

    }

    return 0;
}