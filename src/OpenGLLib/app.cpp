#include "imgui.h"
#include <OpenGLLib/app.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <sstream>

using std::string;
using namespace std;

static App *current = nullptr;

static void global_framebuffer_size_callback(GLFWwindow *window, int w, int h) {
  if (window == current->window) {
    current->framebuffer_size_callback(w, h);
  }
}
static void global_key_callback(GLFWwindow *window, int key, int scancode,
                                int action, int mods) {
  if (window == current->window) {
    current->key_callback(key, scancode, action, mods);
  }
}

static void global_character_callback(GLFWwindow *window,
                                      unsigned int codepoint) {
  if (window == current->window) {
    current->character_callback(codepoint);
  }
}
static void global_cursor_position_callback(GLFWwindow *window, double xpos,
                                            double ypos) {
  if (window == current->window) {
    current->cursor_position_callback(xpos, ypos);
  }
}

static void global_mouse_button_callback(GLFWwindow *window, int button,
                                         int action, int mods) {
  if (window == current->window) {
    current->mouse_button_callback(button, action, mods);
  }
}
static void global_cursor_enter_callback(GLFWwindow *window, int entered) {
  if (window == current->window) {
    current->cursor_enter_callback(entered);
  }
}

static void global_scroll_callback(GLFWwindow *window, double xoffset,
                                   double yoffset) {
  if (window == current->window) {
    current->scroll_callback(xoffset, yoffset);
  }
}

static void global_error_callback(int error, const char* desc) {
  std::cerr << "GFLW Error " << error << ": " << desc << "\n";
}

App::App(unsigned int glmajor, unsigned int glminor, int width, int height,
         const string &title) {
  glfwSetErrorCallback(global_error_callback);
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glmajor);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glminor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
  GLFWmonitor*   monitor = NULL;
  int            w = width, h = height;

  // Si pides “tamaño <=0”, arrancamos en fullscreen
  if (width <= 0 || height <= 0) {
    monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    w = mode->width;
    h = mode->height;

    // Opcional: igualar bits y refresco
    glfwWindowHint(GLFW_RED_BITS,    mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS,  mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS,   mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  }

  window = glfwCreateWindow(w, h, title.c_str(), monitor, NULL);

  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(window);
  // deshabilita V-Sync y permite render loop sin límite de fps
  glfwSwapInterval(0);
  
  primaryMonitor_ = glfwGetPrimaryMonitor();
  vidMode_        = glfwGetVideoMode(primaryMonitor_);
  glfwGetWindowPos(window, &winedX_, &winedY_);
  glfwGetWindowSize(window, &winedW_, &winedH_);
  glfwGetWindowSize(window, &width_, &height_);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    exit(-1);
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  io = &ImGui::GetIO();
  (void)io;
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();
  
  glfwSetFramebufferSizeCallback(window, global_framebuffer_size_callback);
  glfwSetKeyCallback(window, global_key_callback);
  glfwSetMouseButtonCallback(window, global_mouse_button_callback);
  glfwSetCursorPosCallback(window, global_cursor_position_callback);
  glfwSetScrollCallback(window, global_scroll_callback);

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  stringstream ss;
  ss << "#version " << glmajor << glminor << "0";
  ImGui_ImplOpenGL3_Init(ss.str().c_str());
  glEnable(GL_DEPTH_TEST);
}

App::~App() { 
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate(); 
}

bool App::isFinished() { return glfwWindowShouldClose(window); }

void App::render() {
  glViewport(0, 0, width_, height_);
  glClearColor(.2f, .2f, .2f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::update(float deltaTime) {}

void App::framebuffer_size_callback(int width, int height) {
  height_ = height;
  width_ = width;
}
void App::key_callback(int key, int scancode, int action, int mods) {}
void App::character_callback(unsigned int codepoint) {}
void App::cursor_position_callback(double xpos, double ypos) {}
void App::mouse_button_callback(int button, int action, int mods) {}
void App::cursor_enter_callback(int entered) {}
void App::scroll_callback(double xoffset, double yoffset) {}

void App::run() {
  current = this;
  float lastFrame = 0.f;
  while (!isFinished()) {
    float currentFrame = glfwGetTime();
    update(currentFrame - lastFrame);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    render();
    ImGui::Render();
    lastFrame = currentFrame;
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  current = nullptr;
}

void App::toggleFullscreen() {
  if (!isFullscreen_) {
    // vamos a fullscreen

    glfwSetWindowMonitor(
      window,
      primaryMonitor_,
      0, 0,
      vidMode_->width,
      vidMode_->height,
      vidMode_->refreshRate
    );
  } else {
    // volvemos a windowed
    glfwSetWindowMonitor(
      window,
      NULL,
      winedX_, winedY_,
      winedW_, winedH_,
      0
    );
  }
  isFullscreen_ = !isFullscreen_;
}
