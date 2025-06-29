#version 330 core
in vec3 color;
out vec4 fragColor;


void main() {
  vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
  if (dot(circCoord, circCoord) > 1.0) {
    discard;
  }
  fragColor = vec4(color, 1.0);
}
