#include <glcore/app.h>
#include <glcore/buffer.h>
#include <glcore/shader.h>
#include <glm/glm.hpp>
#include <memory>

class MyApp : public App {

public:
  MyApp() : App(4, 0, 800, 600, "Subroutines") {
    shader = std::make_unique<Shader>(SHADER_PATH "vertex.glsl",
                                      SHADER_PATH "fragment.glsl");
    std::vector<float> data = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                               0.0f,  0.0f,  0.5f, 0.0f};
    buff = std::make_unique<BasicBuffer>(data);
    buff->build();
    shader->use();
    redIndex = shader->getSubroutine(GL_FRAGMENT_SHADER, "Euclidean");
    greenIndex = shader->getSubroutine(GL_FRAGMENT_SHADER, "Manhattan");
  }


private:
  std::unique_ptr<Shader> shader;
  std::unique_ptr<Buffer> buff;
  unsigned int redIndex, greenIndex, current = 0;
  float time = 0.f;

  void render() override {
    App::render();
    glDisable(GL_DEPTH_TEST); // solo 2D
    ImGui::Begin("Subroutine Selector");
    if (ImGui::RadioButton("Red", current == 0)) {
      current = 0;
    }
    if (ImGui::RadioButton("Green", current == 1)) {
      current = 1;
    }
    ImGui::End();

    std::vector<unsigned int> indices;
    if (current == 0) {
      indices.push_back(redIndex);
    } else {
      indices.push_back(greenIndex);
    }
    shader->use();
    shader->set("time", time);
    shader->set("resolution", glm::vec2(800.0f, 600.0f));
    shader->setSubroutine(GL_FRAGMENT_SHADER, indices);

    buff->draw();
  }

  void update(float deltaTime) override {
    time += deltaTime;
  }
};

int main() {
  MyApp app;
  app.run();
  return 0;
}
