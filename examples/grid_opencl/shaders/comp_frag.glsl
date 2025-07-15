// comp_packed_pointlight.glsl
#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uPacked;   // RG: R=fuente, G=alive
uniform vec2       uGridSize; // (1/cols,1/rows)
uniform float      uRadius;   // radio en número de celdas (p.ej. 5.0)
uniform float      uIntensity;  // factor de “exposure” o ganancia

void main(){
  // --- Cálculo de la luz puntual ---
  float sum = 0.0;
  int R = int(ceil(uRadius));
  for(int dy = -R; dy <= R; ++dy){
    for(int dx = -R; dx <= R; ++dx){
      vec2 off = vec2(dx, dy) * uGridSize;
      float dist = length(vec2(dx,dy));
      if(dist > uRadius) continue;
      // muestreamos sólo el canal R (fuentes)
      float src = texture(uPacked, vUV + off).r;
      // atenuación inverso al cuadrado + 1 para evitar división por cero
      sum += src / (dist*dist + 1.0);
    }
  }
  // opcionalmente normalizas para no saturar, p.ej.:
  float light = sum / 4;  
  light *= uIntensity;
  light = clamp(light, 0.0, 1.0);

  // --- Estado de la célula (canal G) ---
  float alive = texture(uPacked, vUV).g;

  // --- Colores de resultado ---
  vec3 bg        = vec3(0.0);            // fondo negro
  vec3 ambientC  = vec3(0.5);            // gris para vivas
  vec3 lightC    = vec3(1.0, 0.8, 0.2);   // amarillo suave

  // 1) fondo
  vec3 color = bg;
  // 2) ambient gris sólo en vivas
  color += ambientC * alive * 0.2;  
  // 3) glow amarillo según la luz (también sólo vivas)
  color += lightC * light;

  FragColor = vec4(color, 1.0);
}
