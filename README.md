# Tarea: Interoperabilidad con el Juego de la Vida de Conway

Este repositorio implementa una versión gráfica e interactiva del Juego de la Vida de Conway usando OpenCL y OpenGL. Permite alternar la simulación entre CPU y GPU, cambiar en tiempo real distintos parámetros de la simulación y mostrar celdas con distintos efectos visuales.

---

## Contenido

* [Características Principales](#características-principales)
* [Requisitos](#requisitos)
* [Submódulos](#submódulos)
* [Compilación](#compilación)
* [Ejecución](#ejecución)
* [Controles e Interfaz](#controles-e-interfaz)

---

## Características Principales

* **Modelos de célula arbitrarios**: tres shaders diferentes que cambian en tiempo real (forma, textura, efectos 3D).
* **Iluminación y sombreado**: sombreado de tipo Phong aplicado a las células.
* **Células emisivas**: algunas células emiten luz; puede ajustarse el porcentaje de células emisivas y la intensidad de brillo.
* **Interactividad con el ratón**: añadir o eliminar células con clic izquierdo o derecho.
* **Parámetros en tiempo real**: velocidad de simulación, pausa/continuar, tamaño de la grilla, elección entre procesamiento en CPU o GPU (OpenCL).

---

## Requisitos

1. **C++**: compilador GNU g++ 14.2.0 (o superior).
2. **CMake**: versión 3.31 o superior.
3. **OpenCL**: SDK instalado y configurado en el sistema.
4. **OpenGL**: librerías de desarrollo (GLFW, GLAD).
5. **Git**: para clonar submódulos.

> **Windows (MSYS2)**: puede instalar g++, CMake y librerías con `pacman -Syu base-devel mingw-w64-ucrt/x86_64-{gcc,cmake,glfw,glad}`.
> **Linux (Ubuntu/Debian)**: `sudo apt update && sudo apt install build-essential cmake ocl-icd-opencl-dev libglfw3-dev libglad-dev git`

---

## Submódulos

Este proyecto utiliza repositorios externos como submódulos (OpenGLLib, GameOfLife, etc.). Para clonar correctamente:

```bash
git clone --recursive https://github.com/Vicentendistes/OpenGL-Examples.git
```

---

## Compilación

Usamos CMakePresets para configurar y compilar únicamente la variante OpenCL.

1. Desde la raíz del proyecto, invocar preset `opencl`:

   ```bash
   cmake --preset opencl
   ```
2. Compilar:

   ```bash
   cmake --build --preset opencl
   ```

> Esto generará el ejecutable build/opencl/examples/grid_opencl.exe  # (Windows)


---

## Ejecución

Ejecuta el siguiente comando desde la carpeta raíz:

```bash
./build/opencl/examples/grid_opencl.exe
```


---

## Controles e Interfaz

La ventana principal muestra el simulador y un panel de controles (`ImGui`):

1. **Teclas y acciones globales**:

   * `Space` : Pausar / reanudar la simulación.
   * `F11`   : Alternar pantalla completa.
   * `Esc`   : Cerrar la aplicación.

2. **Panel "Controles"**:

   * **Step**    : Avanza un paso de la simulación.
   * **Restart** : Reinicia la grilla con la configuración inicial.
   * **Random**  : Genera un patrón aleatorio.
   * **Auto Run**: Activar/desactivar ejecución continua.
   * **Velocidad (step/seg)**: Slider para ajustar la velocidad de iteración (sin limite en el máximo).
   * **Interoperabilidad**: Checkbox para alternar CPU ↔ GPU.
   * **Filas / Columnas**: Inputs para redimensionar la grilla.
   * **Redimensionar**: Botón que aplica los nuevos valores de tamaño.
   * **Shader**       : Selector de shader (0, 1 o 2) para cambiar el modelo de célula.

     * *Shader 0*: Modelo básico.
     * *Shader 1*: Modelo con luz direccional (ajustable con deslizador de posición de luz).
     * *Shader 2*: Efectos avanzados de emisión (radio e intensidad ajustables).
   * **Emissive %**: Slider para ajustar el porcentaje de células emisivas.
   * **Base Color** : Selector de color base de las células.
   * **Light Pos**  : (solo Shader 1) Control de posición de la luz.
   * **Radius %** y **Intensity %**: (solo Shader 2) Ajustan radio e intensidad de la emisión.

---
