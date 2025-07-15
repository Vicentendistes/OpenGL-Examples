#version 330 core

layout(location=0) in vec2 aPos;      
layout(location=1) in vec2 aOffset;   
layout(location=2) in int  aState;    

flat out int vState;
out vec3  vFragPos;
out vec3  vNormal;

void main(){
    vec2 pos = aPos + aOffset;
    gl_Position = vec4(pos, 0.0, 1.0);

    vState   = aState;
    vFragPos = vec3(pos, 0.0);
    vNormal  = vec3(0.0, 0.0, 1.0);
}
