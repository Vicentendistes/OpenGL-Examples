#include <glad/glad.h>
#include <OpenGLLib/app.h>
#include <OpenGLLib/buffer.h>   // declara InstanceBuffer
#include <OpenGLLib/shader.h>
#include <GameOfLife/game_of_life_serial.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <iostream>

using uint = unsigned int;


class MyApp : public App {
  //1920x1080

  static constexpr int COLS = 1920;//80;
  static constexpr int ROWS = 1080;//60;

  const float cellW;
  const float cellH;
  
  GameOfLifeSerial game;
  
  std::unique_ptr<Shader> shader;
  std::unique_ptr<InstanceBuffer> buff;
  
  struct Settings {
    bool autoRun = false;
    int stepVel = 5;
    float accumulator = 0.0f;
    float xmouse, ymouse;
    bool leftPressed = false;
    bool rightPressed = false;
    uint lastI = UINT_MAX, lastJ = UINT_MAX;
  } settings;

  

public:
  MyApp()
    : App(3,3,800,600,"Conway Instanced")
    , cellW(2.0f/COLS)
    , cellH(2.0f/ROWS)
    , game(ROWS, COLS)
    , shader(nullptr)
    , buff(nullptr)
    {
    initShaders();
    initGeometry();
    initGame();
  }

private:

  void initShaders();
  void initGeometry();
  void initGame();
  void renderUI();
  
  void render() override {
    App::render();
    glDisable(GL_DEPTH_TEST);

    renderUI();

    // Lógica de stepping automático
    static double lastTime = glfwGetTime();
    double now = glfwGetTime();
    float delta = float(now - lastTime);
    lastTime = now;

    if (settings.autoRun) {
      settings.accumulator += delta;
      if (settings.accumulator >= 1/float(settings.stepVel)) {
        game.step();
        settings.accumulator = 0.0f;
      }
    }

    // Renderizar
    shader->use();
    buff->setStateData(game.getGrid(), COLS*ROWS);
    buff->draw();
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
        game.setCell(i, j, 1);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        game.setCell(i, j, 0);
    }

    settings.lastI = i;
    settings.lastJ = j;
}
};

// ======================
// Definiciones de MyApp
// ======================

void MyApp::initShaders() {
  shader = std::make_unique<Shader>(
    SHADER_PATH "vertex.glsl",
    SHADER_PATH "fragment.glsl"
  );
}

void MyApp::initGeometry() {
  // Usa los miembros cellW, cellH, COLS y ROWS
  float halfW = cellW * 0.5f ;//* 0.9f;
  float halfH = cellH * 0.5f ;//* 0.9f;

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

  buff = std::make_unique<InstanceBuffer>(quadVerts, translations);
  buff->build();
}

void MyApp::initGame() {
  game.initialize();
}

void MyApp::renderUI() {
  ImGui::Begin("Controles");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
  if (ImGui::Button("Step")){
    game.step();
  }    
  if (ImGui::Button("Restart")){
    game.initialize();
  }
  if (ImGui::Button("Random")){
    game.initializeRandom();
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto Run",       &settings.autoRun);
  static int uiSpeed = 0;
  ImGui::SliderInt("Velocidad (step/seg)",
                    &uiSpeed, 0, 21, uiSpeed < 21 ? "%d": "INF");
  settings.stepVel = (uiSpeed < 21 ? uiSpeed : INT_MAX);
  ImGui::End();
}

int main() {
  MyApp app;
  app.run();
  return 0;
}
