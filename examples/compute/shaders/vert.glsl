#version 430 core

layout (std430, binding = 0) buffer particles {
  float data[];
};

uniform mat4 projection;
uniform mat4 view;

out vec3 color;

void main() {
  float x = data[4*gl_VertexID];
  float y = data[4*gl_VertexID+1];
  float vx = data[4*gl_VertexID+2];
  float vy = data[4*gl_VertexID+3];
  vec2 aPos = vec2(x, y);
  vec2 aVel = vec2(vx, vy);
  float speed = length(aVel);
  gl_Position = projection * view * vec4(aPos, 0.f, 1.f);
  color = vec3(speed/2.f, 0.4f, 0.4f);
  // if (outline) {
  //   vec3 pos = aPos * 1.1f;
  //   gl_Position = projection * view * model * vec4(pos, 1.0);
  // } else {
  //   gl_Position = projection * view * model * vec4(aPos, 1.0);
  // } 
}
