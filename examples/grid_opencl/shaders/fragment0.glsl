#version 330 core
flat in int vState;
out vec4 FragColor;

void main(){
    FragColor = (vState == 1)
      ? vec4(1.0,1.0,1.0,1.0)
      : vec4(0.0,0.0,0.0,1.0);
}
