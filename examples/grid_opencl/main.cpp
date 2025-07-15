#include <glad/glad.h>
#include <OpenGLLib/app.h>
#include <OpenGLLib/buffer.h>
#include <OpenGLLib/shader.h>
#include <GameOfLife/game_of_life_opencl.hpp>
#include <GameOfLife/game_of_life_serial.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <iostream>
#include "light_map.h"

using uint = unsigned int;

LightMapPass lightPass;


class MyApp : public App {
  //1920x1080

  int COLS = 192;//1920;
  int ROWS = 108;//1080;

  float cellW;
  float cellH;
  
  std::unique_ptr<GameOfLifeSerial>   gameSerial;
  std::unique_ptr<GameOfLifeOpenCL>   gameOpenCL;
  GameOfLife*                         game; 
  
  std::vector<std::unique_ptr<Shader>> shaders;  
  int shaderIndex = 0;   

  std::unique_ptr<InstanceBuffer> buffCPU;
  std::unique_ptr<InstanceBuffer> buffGPU;

  bool lastParallel = true;
  
  struct Settings {
    bool autoRun = false;
    int stepVel = 5;
    float accumulator = 0.0f;
    float xmouse, ymouse;
    bool leftPressed = false;
    bool rightPressed = false;
    uint lastI = UINT_MAX, lastJ = UINT_MAX;
    bool parallel = true;

    glm::vec3  uLightPos  = glm::vec3( 0.0f, 0.0f, 1.0f );
    float      uRadiantProb = 0.1f;             // 10% células emisivas
    float      uRadius = 5.0f;
    float      uIntensity = 1.0f;
    glm::vec3  uBaseColor   = glm::vec3(0.2f,0.8f,0.2f);

    float cellSize;

  } settings;


public:
  MyApp()
    : App(3,3,1536,864,"Conway Opencl")
    , cellW(2.0f/COLS)
    , cellH(2.0f/ROWS)
    , buffCPU(nullptr)
    , buffGPU(nullptr)
    {

    initShaders();
    initGeometry();
    initGame();
    
    lightPass.setup(width_, height_);
  }

private:
  void initShaders();
  void initGeometry();
  void initGame();
  void renderUI();

  void rebuildEverything();

  void framebuffer_size_callback(int width, int height);

  void render() override {
    App::render();
    glDisable(GL_DEPTH_TEST);

    renderUI();

    
    // Detecta cambio de modo
    if (settings.parallel != lastParallel) {
      ubyte* grid = game->getGrid();
      if (settings.parallel) {
        game = static_cast<GameOfLife*>(gameOpenCL.get());
      }
      else{
        game = static_cast<GameOfLife*>(gameSerial.get());
      }
      game->setGrid(grid);
      lastParallel = settings.parallel;
    }
    
    // stepping automático
    static double lastTime = glfwGetTime();
    double now = glfwGetTime();
    float delta = float(now - lastTime);
    lastTime = now;
    
    if (settings.autoRun) {
      settings.accumulator += delta;
      if (settings.accumulator >= 1/float(settings.stepVel)) {
        game->step();
        settings.accumulator = 0.0f;
      }
    }

    if (shaderIndex !=2){
      // Renderizar
      shaders[shaderIndex]->use();

      shaders[shaderIndex]->set("uLightPos",    settings.uLightPos);
      shaders[shaderIndex]->set("uBaseColor",   settings.uBaseColor);
      shaders[shaderIndex]->set("uRadiantProb",settings.uRadiantProb);
      shaders[shaderIndex]->set("uCellSize", glm::vec2(cellW, cellH));
    }
    
    

    if (settings.parallel){
      if (shaderIndex !=2){
        buffGPU->draw();
      }
      else{
        lightPass.lightShader->use();
        lightPass.lightShader->set("uRadiantProb", settings.uRadiantProb);
        lightPass.renderLightMap(*buffGPU);
        lightPass.composite(glm::vec2(1.0f/COLS, 1.0f/ROWS), settings.uRadius, settings.uIntensity);
      }
    }
    else{
      buffCPU->setStateData(game->getGrid(), COLS*ROWS);
      buffCPU->draw();
    }
  }

  void key_callback(int key, int scancode, int action, int mods) override {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
      settings.autoRun = !settings.autoRun;
    }
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
      toggleFullscreen();
    }
    
  }

  void cursor_position_callback(double xpos, double ypos) override {
    if (io->WantCaptureMouse)
      return;
  
    settings.xmouse = xpos;
    settings.ymouse = ypos;
    // si mantengo pulsado alguno, llamo al handleClick
    if (settings.leftPressed) {
        handleClick(xpos, ypos, GLFW_MOUSE_BUTTON_LEFT);
    }
    else if (settings.rightPressed) {
        handleClick(xpos, ypos, GLFW_MOUSE_BUTTON_RIGHT);
    }

    // shader1
    if (shaderIndex == 1){
      // convierto a NDC:
      float ndcX =  2.0f * xpos / width_  - 1.0f;
      float ndcY = -2.0f * ypos / height_ + 1.0f;
      settings.uLightPos.x = ndcX;
      settings.uLightPos.y = ndcY;
    }
  }

  void mouse_button_callback(int button, int action, int mods) override {
    if (io->WantCaptureMouse)
      return;
    // actualizo flags para cada botón
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        settings.leftPressed = (action == GLFW_PRESS);
        if (settings.leftPressed) {
            // primer “golpe” al pulsar
            handleClick(settings.xmouse, settings.ymouse, GLFW_MOUSE_BUTTON_LEFT);
        }
        else {
            // al soltar, dejo listo para la siguiente celda
            settings.lastI = settings.lastJ = UINT_MAX;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        settings.rightPressed = (action == GLFW_PRESS);
        if (settings.rightPressed) {
            handleClick(settings.xmouse, settings.ymouse, GLFW_MOUSE_BUTTON_RIGHT);
        }
        else {
            settings.lastI = settings.lastJ = UINT_MAX;
        }
    }
  }

  void handleClick(double xpos, double ypos, int button) {
    game->copyGridToHost();
    // convierto a NDC:
    float ndcX =  2.0f * xpos / width_  - 1.0f;
    float ndcY = -2.0f * ypos / height_ + 1.0f;
    uint i = static_cast<uint>((ndcX + 1.0f)*0.5f * COLS);
    uint j = static_cast<uint>((ndcY + 1.0f)*0.5f * ROWS);

    // convierto a idx
    if (i >= COLS || j >= ROWS) return;
    if (i == settings.lastI && j == settings.lastJ) return;

    // LEFT → vivo, RIGHT → muerto
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        game->setCell(i, j, 1);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        game->setCell(i, j, 0);
    }

    settings.lastI = i;
    settings.lastJ = j;
}
};

// ======================
// Definiciones de MyApp
// ======================

void MyApp::initShaders() {
  shaders.emplace_back(std::make_unique<Shader>(
    SHADER_PATH "vertex0.glsl",
    SHADER_PATH "fragment0.glsl"
  ));
  shaders.emplace_back(std::make_unique<Shader>(
    SHADER_PATH "vertex1.glsl",
    SHADER_PATH "fragment1.glsl"
  ));
  shaders.emplace_back(std::make_unique<Shader>(
    SHADER_PATH "vertex2.glsl",
    SHADER_PATH "fragment2.glsl"
  ));
}

void MyApp::initGeometry() {
  // Usa los miembros cellW, cellH, COLS y ROWS
  float halfW = cellW * 0.5f * 0.9f;
  float halfH = cellH * 0.5f * 0.9f;

  std::vector<float> quadVerts = {
    -halfW,  halfH,
     halfW, -halfH,
    -halfW, -halfH,

    -halfW,  halfH,
     halfW, -halfH,
     halfW,  halfH,
  };

  std::vector<glm::vec2> translations;
  translations.reserve(COLS * ROWS);
  for (int y = 0; y < ROWS; ++y) {
    for (int x = 0; x < COLS; ++x) {
      translations.emplace_back(
        -1.0f + (x + 0.5f) * cellW,
        -1.0f + (y + 0.5f) * cellH
      );
    }
  }

  buffCPU = std::make_unique<InstanceBuffer>(quadVerts, translations);
  buffGPU = std::make_unique<InstanceBuffer>(quadVerts, translations);
  buffCPU->build();
  buffGPU->build();
}

void MyApp::initGame() {
  gameSerial = std::make_unique<GameOfLifeSerial>(ROWS, COLS);
  gameOpenCL = std::make_unique<GameOfLifeOpenCL>(ROWS, COLS);
  if (buffGPU){
    gameOpenCL->attachStateVBO(buffGPU->getStateVBO());
  }
  gameSerial->initialize();
  gameOpenCL->initialize();
  if (settings.parallel) {
    game = gameOpenCL.get();
  }
  else {
    game = gameSerial.get();
  }
}

void MyApp::renderUI() {
  ImGui::Begin("Controles");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
  if (ImGui::Button("Step")){
    game->step();
  }    
  if (ImGui::Button("Restart")){
    game->initialize();
  }
  if (ImGui::Button("Random")){
    game->initializeRandom();
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto Run", &settings.autoRun);
  static int uiSpeed = 0;
  ImGui::SliderInt("Velocidad (step/seg)",
    &uiSpeed, 0, 21, uiSpeed < 21 ? "%d": "INF");
  settings.stepVel = (uiSpeed < 21 ? uiSpeed : INT_MAX);

  ImGui::Checkbox("Interoperabilidad", &settings.parallel);

  // sliders para tamaño
  static int uiRows = ROWS, uiCols = COLS;
  if (ImGui::InputInt("Filas", &uiRows) &
      ImGui::InputInt("Columnas", &uiCols)) {
    // solo actualizar variables, espera a botón
  }
  if (ImGui::Button("Redimensionar")) {
    ROWS = std::max(1, uiRows);
    COLS = std::max(1, uiCols);
    rebuildEverything();
  }

  const char* items[] = { "Shader 0", "Shader 1", "Shader 2"};
  ImGui::Combo("Shader", &shaderIndex, items, IM_ARRAYSIZE(items));

  if (shaderIndex !=0){
    ImGui::SliderFloat("Emissive %", &settings.uRadiantProb, 0.0f, 1.0f);
    ImGui::ColorEdit3 ("Base Color", reinterpret_cast<float*>(&settings.uBaseColor));
    if (shaderIndex == 1){
      ImGui::SliderFloat3("Light Pos", reinterpret_cast<float*>(&settings.uLightPos), 0,5);
    }
    else{
      ImGui::SliderFloat("Radius %", &settings.uRadius, 0.0f, 10.0f);
      ImGui::SliderFloat("Intensity %", &settings.uIntensity, 1.0f, 3.0f);
    }
  }

  

  ImGui::End();
}

void MyApp::rebuildEverything() {

  // 1) Destruye primero la simulación OpenCL (libera glStateBuffer)
  gameOpenCL.reset();

  // 2) Destruye el VBO de OpenCL
  buffGPU.reset();

  // 3) Ahora destruye lo demás
  gameSerial.reset();
  buffCPU.reset();

  cellW = 2.0f / COLS;
  cellH = 2.0f / ROWS;
  initGeometry();
  initGame();
}

void MyApp::framebuffer_size_callback(int width, int height){
  App::framebuffer_size_callback(width, height);
  lightPass.resize(width, height);
}

int main() {
  MyApp app;
  app.run();
  return 0;
}
