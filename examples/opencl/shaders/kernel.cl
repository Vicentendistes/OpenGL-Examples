kernel void update(global float *data, unsigned int size, float xmouse,
                   float ymouse, float deltaTime) {
  int index = 4 * get_global_id(0);
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
    index += 4 * get_global_size(0);
  }
  // data[2*gindex] = cos(time);
  // data[2*gindex + 1] = sin(time);
}
