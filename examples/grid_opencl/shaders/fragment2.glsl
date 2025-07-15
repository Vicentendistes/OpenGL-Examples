// fragment.glsl
#version 330 core

flat in int  vState;
in  vec3     vFragPos;
in  vec3     vNormal;

uniform vec3  uLightPos;       // desde tu GUI
uniform float uRadiantProb;    // [0..1] desde tu GUI
uniform vec3  uBaseColor;      // desde tu GUI

out vec4 FragColor;

// hash sencillo para un rand reproducible por celda
float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

void main(){
    if(vState == 0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else{
        float r = rand(vFragPos.xy);
        vec3 emissive = (r < uRadiantProb)
                    ? (r / uRadiantProb) * vec3(1.0,1.0,0.0)
                    : vec3(0.0);

        vec3 color = emissive;
        FragColor = vec4(color, 1.0);
    }
}
