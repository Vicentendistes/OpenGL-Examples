#include <cstdlib>
#include <OpenGLLib/camera.h>
#include <OpenGLLib/core.h>
#include <OpenGLLib/texture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class MyApp : public App {
  bool firstMouse = true;
  double lastX_ = 0.0, lastY_ = 0.0;
public:
  MyApp() : App(3, 3, 800, 600, "Textures") {

    lastX_ = width_ / 2.0;
    lastY_ = height_ / 2.0;

    shader = std::make_unique<Shader>(SHADER_PATH "vertex.glsl",
                                      SHADER_PATH "fragment.glsl");

    std::vector<float> data(
        {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
         -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

         -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

         -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
         -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
         0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
         0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
         0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
         -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

         -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f});
    buf = std::make_unique<TextureBuffer>(data);

    settings.container = textures.size();
    textures.emplace_back(RESOURCES_PATH "container.jpg");
    settings.face = textures.size();
    textures.emplace_back(RESOURCES_PATH "awesomeface.png");
    
    // Texture *container = new Texture(RESOURCES_PATH "container.jpg");
    // Texture *face = new Texture(RESOURCES_PATH "awesomeface.png");

    buf->addTexture(&textures[settings.container]);
    buf->addTexture(&textures[settings.face]);
     
    // buf->addTexture(container);
    // buf->addTexture(face);
    
    buf->build();

    cam = std::make_unique<Camera>();
    cam->position = glm::vec3(-5.f, 0.f, 0.f);
    cam->speed = 2.f;
    cam->distance = 2.f;

    positions = std::vector<glm::vec3>(
        {glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 5.0f, -15.0f),
         glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
         glm::vec3(2.4f, -0.4f, -3.5f), glm::vec3(-1.7f, 3.0f, -7.5f),
         glm::vec3(1.3f, -2.0f, -2.5f), glm::vec3(1.5f, 2.0f, -2.5f),
         glm::vec3(1.5f, 0.2f, -1.5f), glm::vec3(-1.3f, 1.0f, -1.5f)});

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  void key_callback(int key, int scancode, int action, int mods) override {
    if (action == GLFW_PRESS) {
      if (key == GLFW_KEY_W) {
        settings.ycam += 1.f;
      }
      if (key == GLFW_KEY_S) {
        settings.ycam -= 1.f;
      }
      if (key == GLFW_KEY_D) {
        settings.xcam += 1.f;
      }
      if (key == GLFW_KEY_A) {
        settings.xcam -= 1.f;
      }
      if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, 1);
    }
    if (action == GLFW_RELEASE) {
      if (key == GLFW_KEY_W) {
        settings.ycam -= 1.f;
      }
      if (key == GLFW_KEY_S) {
        settings.ycam += 1.f;
      }
      if (key == GLFW_KEY_D) {
        settings.xcam -= 1.f;
      }
      if (key == GLFW_KEY_A) {
        settings.xcam += 1.f;
      }
    }
    cam->key_callback(settings.xcam, settings.ycam);
  }

  void cursor_position_callback(double xpos, double ypos) override {
    if (firstMouse) {
      lastX_ = xpos;
      lastY_ = ypos;
      firstMouse = false;
    }

    // calcula offset desde la última posición
    float xoffset = float(xpos - lastX_);
    float yoffset = float(lastY_ - ypos); // invertido para que Y crezca hacia arriba
    lastX_ = xpos;
    lastY_ = ypos;
  
    cam->cursor_position_callback(xoffset, yoffset);
  }

  void scroll_callback(double xoffset, double yoffset) override {
    cam->scroll_callback(yoffset);
  }

private:
  struct {
    float xcam = 0.f, ycam = 0.f;
    int container, face;
  } settings;

  std::unique_ptr<Shader> shader;
  std::unique_ptr<Camera> cam;
  std::unique_ptr<TextureBuffer> buf;
  std::vector<Texture> textures;
  std::vector<glm::vec3> positions;

  void render() override {
    App::render();
    shader->use();
    shader->set("view", cam->view());
    shader->set("projection",
                glm::perspective(glm::radians(cam->zoom),
                                 ((float)width_) / ((float)height_), 0.1f,
                                 100.0f));
    shader->set("tex1", 0);
    shader->set("tex2", 1);
    int i = 0;
    for (auto &pos : positions) {
      glm::mat4 model(1.0f);
      model = glm::translate(model, pos);
      float angle = 20.0f * i;
      i++;
      model =
          glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      shader->set("model", model);
      buf->draw();
    }
  }

  void update(float deltaTime) override { cam->update(deltaTime); }
};

int main() {
  MyApp app;
  app.run();
  return 0;
}
