// comp_frag.glsl (ajustado)
#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uPacked;  // si usas la versión empaquetada RG16F
uniform vec2       uGridSize;

void main(){
  // --- blur 3×3 sobre canal R (fuentes) ---
  // Kernel Gaussiano 3×3
    float kernel[9] = float[](
        1, 2, 1,
        2, 4, 2,
        1, 2, 1
    );
  float sum = 0.0;
  int idx = 0;
  for(int y=-1; y<=1; ++y){
    for(int x=-1; x<=1; ++x){
      vec2 off = vec2(x,y)*uGridSize;
      sum += texture(uPacked, vUV + off).r * kernel[idx++];
    }
  }
  float light = sum / 8;

  // --- estado vivo en canal G ---
  float alive = texture(uPacked, vUV).g; // 1.0 vivas, 0.0 muertas

  // --- colores base ---
  vec3 bgColor    = vec3(0.0);           // fondo negro
  vec3 cellGray   = vec3(0.6);           // gris claro para vivas sin luz
  vec3 lightColor = vec3(1.0, 0.8, 0.2); // amarillo suave

  // --- mezcla final ---
  // 1) start con fondo negro
  vec3 color = bgColor;
  // 2) añadir gris donde haya célula viva (ambient)
  color += cellGray * alive * 0.2;       // 0.2 es factor ambiental
  // 3) añadir luz amarilla sólo sobre vivas
  color += lightColor * light;

  FragColor = vec4(color, 1.0);
}
