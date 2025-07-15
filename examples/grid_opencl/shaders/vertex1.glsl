#version 330 core

layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aOffset;
layout(location=2) in int  aState;

flat out int  vState;
flat out int  vRadiant;    // 0 o 1
out      vec3 vFragPos;
out      vec3 vNormal;

uniform float uRadiantProb; // % de células que emiten

float rand(vec2 co){
    return fract(sin(dot(co,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){
    vec2 pos = aPos + aOffset;
    gl_Position = vec4(pos, 0.0, 1.0);

    vFragPos = vec3(pos, 0.0);
    vNormal  = vec3(0.0,0.0,1.0);
    vState   = aState;
    // decide si esta célula es fuente de luz
    vRadiant = (aState == 1 && rand(aOffset) < uRadiantProb) ? 1 : 0;
}
