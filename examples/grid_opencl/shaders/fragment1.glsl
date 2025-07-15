#version 330 core

flat in int  vState;
flat in int  vRadiant;
in  vec3     vFragPos;
in  vec3     vNormal;

uniform float uRadiantProb;  // de tu GUI
uniform vec3  uBaseColor;    // de tu GUI
uniform vec2  uCellSize;     // = vec2(cellW,cellH) de tu GUI
uniform vec3  uLightPos;       // desde tu GUI

out vec4 FragColor;

void main(){
    if(vState == 0) {
      FragColor = vec4(0.0,0.0,0.0,1.0);
      return;
    }
    // 1) Ambient
    vec3 ambient = 0.1 * uBaseColor;

    // 2) Diffuse (Phong)
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightPos - vFragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * uBaseColor;

    // 3) Specular
    vec3 V = vec3(0.0,0.0,1.0);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0);

    vec3 color = ambient + diffuse + specular;
    
    // 4) Emissive propia
    if(vRadiant == 1){
      color += vec3(1.0,1.0,0.0) * 0.5;
    }

    vec2 uv = fract(vFragPos.xy / uCellSize) * 2.0 - 1.0;  // [-1..1] centrado
    float d = length(uv);
    float grad = smoothstep(1.0, 0.0, d);                 // 1 en centro, 0 en borde
    color *= grad;                                        // atenúa en los bordes



    FragColor = vec4(color,1.0);
}
