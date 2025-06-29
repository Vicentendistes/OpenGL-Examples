#version 400 core

uniform float time;
uniform vec2 resolution;

const int MAX_DIST_ARGS = 4;

// Subroutine declaration
subroutine float distanceFunction(vec3 a, vec3 b, float args[MAX_DIST_ARGS]);
subroutine uniform distanceFunction chosenDistanceFunction;

// Euclidean distance function
subroutine(distanceFunction)
float Euclidean(vec3 a, vec3 b, float args[MAX_DIST_ARGS]) {
    return length(a - b);
}

// Manhattan distance function
subroutine(distanceFunction)
float Manhattan(vec3 a, vec3 b, float args[MAX_DIST_ARGS]) {
    return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
}

// Function to generate hash values
vec3 hash3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453);
}

// Function to generate cellular noise
float cellularNoise(vec3 P) {
    vec3 Pi = floor(P);
    vec3 Pf = fract(P);

    float F1 = 1.0;
    float F2 = 1.0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                vec3 neighbor = vec3(float(x), float(y), float(z));
                vec3 point = hash3(Pi + neighbor);

                vec3 diff = neighbor + point - Pf;
                float dist = chosenDistanceFunction(neighbor + point, Pf, float[MAX_DIST_ARGS](0.0, 0.0, 0.0, 0.0));
                //float dist = length(diff);
                if (dist < F1) {
                    F2 = F1;
                    F1 = dist;
                } else if (dist < F2) {
                    F2 = dist;
                }
            }
        }
    }
    return F2 - F1;
}

// Fragment shader entry point
out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec3 pos = vec3(uv * 10.0, time * 0.1);
    float noise = cellularNoise(pos);
    vec3 color = vec3(noise);

    outColor = vec4(color, 1.0);
}

