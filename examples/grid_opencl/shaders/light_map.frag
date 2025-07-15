// light_map.frag
#version 330 core
flat in int vRadiant;
flat in int vState;
layout(location=0) out vec2 outData; // R=fuente, G=estado

void main(){
  float src = (vRadiant==1 ? 1.0 : 0.0);
  float st  = (vState  ==1 ? 1.0 : 0.0);
  outData = vec2(src, st);
}
