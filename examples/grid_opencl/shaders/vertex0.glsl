#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aOffset;
layout(location=2) in int aState;

flat out int vState;

void main(){
    vState = aState;
    gl_Position = vec4(aPos + aOffset, 0.0, 1.0);
}
