#pragma once
#include <glm/glm.hpp>

struct Settings
{
    unsigned int amount = 4e2;               // Number of boids
    glm::vec2 screenSize = {800.0f, 600.0f}; // Screen size

    float boidSize = 10.0f; // Size of each boid

    float maxSpeed = 500.0f; // Maximum speed of boids
    float maxForce = 200.0f; // Maximum steering force

    int simulationType = 0; // 0 for CPU, 1 for GPU

    float alignmentWeight = 1.0f;  // Alignment weight
    float cohesionWeight = 1.0f;   // Cohesion weight
    float separationWeight = 1.0f; // Separation weight

    float angleView = 360.0f;       // View angle in degrees
    float perceptionRadius = 50.0f; // Perception radius for neighbors
    float avoidanceRadius = 20.0f;  // Avoidance radius for obstacles
};
