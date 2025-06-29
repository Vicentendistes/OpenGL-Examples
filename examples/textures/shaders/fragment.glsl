#version 330 core
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {
  fragColor = mix(texture(tex1, texCoord), texture(tex2, texCoord), 0.2);
}
