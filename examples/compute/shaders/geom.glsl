#version 330 core
layout (points) in;
layout ( triangle_strip, max_vertices=100 ) out;

in float speed[];

out vec3 color;

uniform float size;
uniform mat4 projection;
uniform mat4 view;
uniform int quality;

void main() {
  float deltaTheta = radians(360) / quality;
  for(int i = 0; i <= quality; i++) {
    float x = cos(deltaTheta*i) * size;
    float y = sin(deltaTheta*i) * size;
    gl_Position = projection * view * gl_in[0].gl_Position;
    color = vec3(speed[0]/2.f, 0.4f, 0.4f);
    EmitVertex();
    color = vec3(speed[0]/2.f, 0.4f, 0.4f);
    gl_Position =  projection * view * (gl_in[0].gl_Position + vec4(x, y, 0.0f, 0.0f));
    EmitVertex();
  }
  EndPrimitive();
  // vec3 top = vec3(0.0f, 1.0f, 0.0f) * size;
  // vec3 left = vec3(-1.0f, 0.0f, 0.0f) * size;
  // vec3 right = vec3(1.0f, 0.0f, 0.0f) * size;

  // gl_Position = vec4(top, 1.0f) + pos;
  // EmitVertex();
  // gl_Position = vec4(right, 1.0f) + pos;
  // EmitVertex();
  // gl_Position = vec4(left, 1.0f) + pos;
  // EmitVertex();
  // EndPrimitive();
}
