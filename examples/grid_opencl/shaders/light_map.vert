#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aOffset;
layout(location=2) in int  aState;

flat out int vRadiant;
flat out int vState;

uniform float uRadiantProb;

float rand(vec2 c){ return fract(sin(dot(c,vec2(12.9898,78.233)))*43758.5453); }

void main(){
  vRadiant = (aState==1 && rand(aOffset)<uRadiantProb) ? 1 : 0;
  vState = aState;
  gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
}
