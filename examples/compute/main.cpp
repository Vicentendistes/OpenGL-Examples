#include "glcore/app.h"
#include "imgui.h"
#include <glcore/core.h>
#include <iostream>
#ifndef GL_COMPUTE_SHADER
int main (int argc, char *argv[]) {
  std::cout << "Not supported!\n";
  return 0;
}
#else
#include <memory>
#include <glcore/camera.h>
#include <glm/ext.hpp>

#define RANDOM ((float)rand() / (float)(RAND_MAX))

class StorageBuffer : public Buffer {
public:
  StorageBuffer(int binding, const std::vector<float> &vertices): binding_(binding), vertices_(vertices) {
    glGenBuffers(1, &ssbo_);
  };

  ~StorageBuffer() {
    if (ssbo_)
      glDeleteBuffers(1, &ssbo_);
  }

  void build() override {
    bind();
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * vertices_.size(), vertices_.data(), GL_DYNAMIC_DRAW);
  }

  void draw() override {
    // float temp[vertices_.size()];
    // glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * vertices_.size(), temp);
    // for (int i = 0; i < vertices_.size(); i++) {
    //   std::cout << temp[i] << " ";
    // }
    // std::cout << "\n";
    // bind();
    // glDrawArrays(GL_POINTS, 0, vertices_.size()/4);
  }

  void bind() override {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_, ssbo_);
  }

private:
  std::vector<float> vertices_;
  unsigned int ssbo_;
  int binding_;
};

class MyBuff: public BasicBuffer {
  using BasicBuffer::BasicBuffer;


public:
  void draw() override {
    bind();
    glDrawElements(GL_POINTS, indices_.size(), GL_UNSIGNED_INT, 0);
  };
};

class MyApp: public App {
public:
  MyApp(): App(4, 3, 800, 600, "Compute shaders") {
    glfwSwapInterval(0);
    sh = std::make_unique<Shader>(SHADER_PATH "vert.glsl",
                                  SHADER_PATH "frag.glsl");
    compute = std::make_unique<ComputeShader>(SHADER_PATH "compute.glsl");
    std::vector<float> data;
    std::vector<unsigned int> idx;
    for (int i = 0; i < settings.amount; i++) {
      data.push_back(RANDOM*2.f - 1.0f);
      data.push_back(RANDOM*2.f - 1.0f);
      data.push_back(0.f);
      data.push_back(0.f);
      idx.push_back(i);
    }
    buff = std::make_unique<StorageBuffer>(0, data);
    buff->build();
    ind = std::make_unique<MyBuff>(std::vector<float>(), idx);
    ind->build();
    cam = std::make_unique<Camera>(glm::vec3(0.f, 0.f, 3.f), 1.f, -90.f);
  }

  void render() override {
    App::render();
    glDisable(GL_DEPTH_TEST); // solo 2D
    ImGui::Begin("Information"); // Create a window called "Hello,
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui::SliderFloat("Size", &settings.size, 1.f, 100.f);
    ImGui::SliderInt("Work groups", &settings.global, 1, GL_MAX_COMPUTE_WORK_GROUP_COUNT);
    ImGui::End();
    compute->release();
    glPointSize(settings.size);
    sh->use();
    sh->set("view", cam->view());
    sh->set("projection", projection);
    buff->bind();
    ind->draw();
  }

  void update(float deltaTime) override {
    float aspect = (float)width_ / (float)height_;
    projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);

    float x = (2.0f * settings.xmouse) / width_ - 1.0f;
    float y = 1.0f - (2.0f * settings.ymouse) / height_;

    // from viewport to world
    glm::vec4 pos(x, y, 0.0f, 1.0f);
    glm::mat4 toWorld = glm::inverse(projection * cam->view());
    glm::vec4 realPos = toWorld * pos;

    compute->use();
    compute->global[0] = settings.global;
    compute->set("size", settings.amount);
    compute->set( "xmouse", realPos.x);
    compute->set("ymouse", realPos.y);
    compute->set("deltaTime", deltaTime);
    buff->bind();
    compute->execute();
  }

  void cursor_position_callback(double xpos, double ypos) override {
    if (io->WantCaptureMouse)
      return;
    settings.xmouse = xpos;
    settings.ymouse = ypos;
  }
private:
  std::unique_ptr<ComputeShader> compute;
  std::unique_ptr<Shader> sh;
  std::unique_ptr<StorageBuffer> buff;
  std::unique_ptr<MyBuff> ind;
  std::unique_ptr<Camera> cam;
  glm::mat4 projection;

  struct {
    float xmouse, ymouse;
    unsigned int amount = 1e7;
    float size = 1.f;
    int global = 1024;
  } settings;
};

int main (int argc, char *argv[]) {
  MyApp app;
  app.run();
  return 0;
}
#endif 
