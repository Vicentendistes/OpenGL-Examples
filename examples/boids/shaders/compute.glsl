#version 430 core
layout(local_size_x = 256) in;

struct Boid {
    vec2 position;
    vec2 velocity;
};

layout(std430, binding = 0) buffer Boids {
    Boid boids[];
};
layout(std430, binding = 1) buffer NewBoids {
    Boid newBoids[];
};

uniform float deltaTime;
uniform vec2 screenSize;
uniform uint amount; // number of boids
uniform float maxSpeed;
uniform float maxForce;
uniform float alignmentWeight;
uniform float cohesionWeight;
uniform float separationWeight;
uniform float angleView;
uniform float perceptionRadius;
uniform float avoidanceRadius;

void clampLength(inout vec2 v, float maxLength) {
    float len = length(v);
    if (len > maxLength && len > 0.0)
        v = v * (maxLength / len);
}

vec2 steerTowards(vec2 current, vec2 target, float maxSpeed, float maxForce) {
    vec2 desired = normalize(target) * maxSpeed;
    vec2 steer = desired - current;
    clampLength(steer, maxForce);
    return steer;
}

float angleBetween(vec2 a, vec2 b) {
    return degrees(acos(dot(normalize(a), normalize(b))));
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= amount) return;

    Boid boid = boids[i];
    vec2 alignment = vec2(0.0);
    vec2 cohesion = vec2(0.0);
    vec2 separation = vec2(0.0);
    int flockSize = 0;
    int closeSize = 0;

    for (uint j = 0; j < amount; ++j) {
        if (i == j) continue;
        Boid other = boids[j];
        vec2 toOther = other.position - boid.position;
        float angle = angleBetween(boid.velocity, toOther);
        float dist = length(toOther);

        if (angle > angleView / 2.0) continue;

        if (dist < perceptionRadius) {
            alignment += other.velocity;
            cohesion += other.position;
            flockSize++;
            if (dist < avoidanceRadius) {
                separation -= toOther / (dist * dist);
                closeSize++;
            }
        }
    }

    vec2 acceleration = vec2(0.0);
    if (flockSize > 0) {
        alignment /= float(flockSize);
        cohesion /= float(flockSize);
        cohesion -= boid.position;

        acceleration += steerTowards(boid.velocity, alignment, maxSpeed, maxForce) * alignmentWeight;
        acceleration += steerTowards(boid.velocity, cohesion, maxSpeed, maxForce) * cohesionWeight;
        if (closeSize > 0) {
            acceleration += steerTowards(boid.velocity, separation, maxSpeed, maxForce) * separationWeight;
        }
    }

    // Update velocity and position
    boid.velocity += acceleration * deltaTime;
    clampLength(boid.velocity, maxSpeed);

    boid.position += boid.velocity * deltaTime;

    // Wrap around screen edges
    if (boid.position.x < 0.0)
        boid.position.x += screenSize.x;
    else if (boid.position.x > screenSize.x)
        boid.position.x -= screenSize.x;

    if (boid.position.y < 0.0)
        boid.position.y += screenSize.y;
    else if (boid.position.y > screenSize.y)
        boid.position.y -= screenSize.y;
        

    newBoids[i] = boid;
}
