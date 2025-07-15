#ifndef _APP_H_
#define _APP_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <imgui.h>

class App {
public:
  GLFWwindow* window;
  ImGuiIO*    io;

  App(unsigned int glmajor,
      unsigned int glminor,
      int          width,
      int          height,
      const std::string &title = "App");

  virtual ~App(); 
  void run();

  // Callbacks que implementarás/overrides en tu MyApp
  virtual void framebuffer_size_callback(int width, int height);
  virtual void key_callback(int key, int scancode, int action, int mods);
  virtual void character_callback(unsigned int codepoint);
  virtual void cursor_position_callback(double xpos, double ypos);
  virtual void mouse_button_callback(int button, int action, int mods);
  virtual void cursor_enter_callback(int entered);
  virtual void scroll_callback(double xoffset, double yoffset);
  void toggleFullscreen();

protected:
  virtual bool isFinished();
  virtual void update(float deltaTime);
  virtual void render();
  int height_;
  int width_;

private:
  // ————— Toggle Fullscreen —————
  bool               isFullscreen_   = false;
  int                winedX_      = 0;
  int                winedY_      = 0;
  int                winedW_      = 0;
  int                winedH_      = 0;
  GLFWmonitor*       primaryMonitor_ = nullptr;
  const GLFWvidmode* vidMode_        = nullptr;

};

#endif // _APP_H_
