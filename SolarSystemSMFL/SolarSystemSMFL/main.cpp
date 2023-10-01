#include <iostream>
#include <SFML/Graphics.hpp>
#include <cmath>
#include<array> 
#include <random>

using namespace std;
using namespace sf;

const float GRAVITY_CONSTANT = 2;

// Return squared magnitude
float squaredMagnitude(const Vector2f& a) {
    return a.x * a.x + a.y * a.y;
}

// Return squared distance
float squaredDistance(const Vector2f& a, const Vector2f& b) {
    Vector2f diff = a - b;
    return squaredMagnitude(diff);
}

class SolarObject {
public:
    int mass;
    Vector2f position;
    Vector2f velocity;
    float radius;
    CircleShape shape;

    void UpdateVelocity(SolarObject* solarObjects, int length) {
        const float softening = 3.0f; // You can experiment with this value

        for (int i = 0; i < length; i++) {
            if (&solarObjects[i] == this) continue;

            // Get direction from this object to the other object
            Vector2f direction = solarObjects[i].position - this->position;
            float sqDist = squaredDistance(this->position, solarObjects[i].position);

            // Introduce the softening term to avoid singularity
            float force = GRAVITY_CONSTANT * (1/(sqDist + softening * softening));

            // Normalize direction and multiply by force to get velocity change
            float magnitude = std::sqrt(sqDist);
            direction /= magnitude;
            Vector2f velocityChange = direction * force;
            velocity += velocityChange;
        }
    }

    void UpdatePosition() {
        position += velocity;
    }

    sf::CircleShape& Draw() {
        shape.setPosition(position.x - radius, position.y - radius);
        return shape;
    }

    SolarObject(int mass_ = 100, const sf::Vector2f& position_ = Vector2f(0, 0), const sf::Vector2f& velocity_ = Vector2f(0, 0), float radius_ = 1)
        : mass(mass_), position(position_), velocity(velocity_), radius(radius_) {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(255,255,255,80));
    }
};

float random_float(float min, float max) {
    static std::uniform_real_distribution<float> distribution(min, max);
    static std::mt19937 generator;
    return distribution(generator);
}

int main()
{
    random_device rd;  // A random seed generator
    mt19937 gen(rd());  // Mersenne Twister pseudo-random number generator
    uniform_int_distribution<int> distribution(1, 1000);  // Define the range [1, 100]
    uniform_int_distribution<int> VelDistribution(0, 0);  // Define the range [1, 100]

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "N-Body simulation");

    sf::Clock clock;  // Clock to measure time since the last update
    const sf::Time TimePerFrame = sf::seconds(1.0f / 30.0f);  // 30 updates per second
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    SolarObject planets[800];
    int length = 800;

    const float center_x = 1920/2;
    const float center_y = 1080/2;
    const float initial_radius = 65;         // Start further from the center
    const float spiral_arm_separation = 26;  // Increase distance between spiral arms
    const float angle_increment = 0.004;     // Slightly smaller increment to space out particles more
    const float random_radius_max = 15;      // Increase randomness in the radius
    const float random_angle_max = 0.5; 

    // This constant can be adjusted to change the rotation speed of the particles
    const float rotation_speed_factor = 0.1;

    float current_angle = 0;

    for (int i = 0; i < length; i++)
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

        planets[i] = SolarObject(500, Vector2f(x, y), tangential_velocity, 3);

        current_angle += angle_increment;
    }

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
            
            for (int i = 0; i < length; i++)
            {
                planets[i].UpdateVelocity(planets, length);
            }
            
            for (int i = 0; i < length; i++)
            {
                planets[i].UpdatePosition();
                window.draw(planets[i].Draw());
            }

            window.display();
        }
    }

    return 0;
}