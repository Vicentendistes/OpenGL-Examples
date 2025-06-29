#version 430 core
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer particles {
  float data[];
};

layout (location = 0) uniform uint size;
layout (location = 1) uniform float xmouse;
layout (location = 2) uniform float ymouse;
layout (location = 3) uniform float deltaTime;

void main() {
  uint index = 4 * gl_GlobalInvocationID.x;
  while (index < 4*size) {
    float x = data[index];
    float y = data[index + 1];
    float vx = data[index + 2];
    float vy = data[index + 3];
    // float mass = data[index+4];
    float dx = xmouse - x;
    float dy = ymouse - y;
    float distance = sqrt(dx * dx + dy * dy);
    if (distance > 0.1f) {
      float acceleration = .1f / (distance * distance);
      float xdir = dx / distance;
      float ydir = dy / distance;
      float ax = acceleration * xdir;
      float ay = acceleration * ydir;
      vx += ax * deltaTime;
      vy += ay * deltaTime;
    }
    x += vx * deltaTime;
    y += vy * deltaTime;
    data[index] = x;
    data[index + 1] = y;
    data[index + 2] = vx;
    data[index + 3] = vy;
    index += 4 * gl_WorkGroupSize.x * gl_NumWorkGroups.x;
  }
  // data[2*gindex] = cos(time);
  // data[2*gindex + 1] = sin(time);
}
