#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aVel;

uniform mat4 projection;
uniform mat4 view;

out vec3 color;

void main() {
  gl_Position = projection * view * vec4(aPos, 0.f, 1.f);
  float speed = length(aVel);
  color = vec3(speed/2.f, 0.4f, 0.4f);
  // if (outline) {
  //   vec3 pos = aPos * 1.1f;
  //   gl_Position = projection * view * model * vec4(pos, 1.0);
  // } else {
  //   gl_Position = projection * view * model * vec4(aPos, 1.0);
  // } 
}
