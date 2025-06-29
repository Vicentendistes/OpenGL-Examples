#version 430 core

struct Boid {
    vec2 position;
    vec2 velocity;
};

layout(location = 0) in vec2 position;
out vec4 fragColor;

layout(std430, binding = 0) buffer Boids {
    Boid boids[];
};

uniform float boidSize;
uniform vec2 screenSize;
uniform float maxSpeed;

void main() {
    Boid b = boids[gl_InstanceID];
    vec2 forward = normalize(b.velocity);
    vec2 right = vec2(-forward.y, forward.x);

    mat2 rot = mat2(right, forward);
    vec2 worldPos = rot * (position * boidSize) + b.position;

    vec2 ndc = (worldPos / screenSize) * 2.0 - 1.0;
    ndc.y = -ndc.y;

    gl_Position = vec4(ndc, 0.0, 1.0);

    // Set the color based on the boid's velocity
    float rCol = (b.velocity.x + maxSpeed) / (2.0 * maxSpeed);
    float gCol = (b.velocity.y + maxSpeed) / (2.0 * maxSpeed);
    fragColor = vec4(rCol, gCol, 1.0, 1.0); // Set the color to white
}
