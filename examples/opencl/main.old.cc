#define CL_TARGET_OPENCL_VERSION 120
#if defined (__APPLE__)
#include <OpenCL/opencl.h> // use C standard
#include <OpenGL/OpenGL.h>
#elif defined (__linux__)
#include <CL/opencl.h> // use C standard
#include <GL/gl.h>
#include <GL/glx.h>
#elif defined (_WIN32)
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif
#include <glcore/core.h>
#include <glcore/camera.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define RANDOM ((float)rand() / (float)(RAND_MAX))

class SharedBuffer : public VertexBuffer {
public:
  cl_mem mem;
  SharedBuffer(cl_context context, const std::vector<float> &vertices)
      : VertexBuffer(vertices) {
    bind();
    int err;
    mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, vbo_, &err);
    if (!mem || err != CL_SUCCESS) {
      std::cerr << "Error: failed to create buffer from GL buffer '" << vbo_ << "' (e: " << err
                << ")\n";
    }
  };
  void build() override {
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_.size(),
                 vertices_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }

  void draw() override {
    bind();
    glDrawArrays(GL_POINTS, 0, vertices_.size()/4);
  }

  void acquire(cl_command_queue queue) {
    int err = CL_SUCCESS;
    err = clEnqueueAcquireGLObjects(queue, 1, &mem, 0, 0, 0);
    if (err != CL_SUCCESS) {
        printf("Failed to attach Vertex Buffer!\n");
        return;
    }
  }

  void release(cl_command_queue queue) {
    int err = CL_SUCCESS;
    err = clEnqueueReleaseGLObjects(queue, 1, &mem, 0, 0, 0);
  }

};

class MyApp : public App {
public:
  std::unique_ptr<SharedBuffer> buff;
  std::unique_ptr<Shader> sh;
  std::unique_ptr<Camera> cam;
  glm::mat4 projection;

  struct {
    cl_context context;
    cl_device_id device;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    int max;
  } cl;

  struct {
    float xmouse, ymouse;
    int amount = 10;
    int quality = 10;
    int local = 32;
    int global = 0;
  } settings;

  MyApp() : App(4, 0, 800, 800, "OpenCL interoperabillity") {
    setup_compute();
    sh = std::make_unique<Shader>(SHADER_PATH "vert.glsl",
                                  SHADER_PATH "geom.glsl",
                                  SHADER_PATH "frag.glsl");
    std::vector<float> data;
    for (int i = 0; i < settings.amount; i++) {
      data.push_back(RANDOM*2.f - 1.0f);
      data.push_back(RANDOM*2.f - 1.0f);
      data.push_back(0.f);
      data.push_back(0.f);
    }
    buff = std::make_unique<SharedBuffer>(cl.context, data);
    buff->build();
    cam = std::make_unique<Camera>(glm::vec3(0.f, 0.f, 3.f), 1.f, -90.f);
  }
  void render() override {
    App::render();
    glDisable(GL_DEPTH_TEST); // solo 2D
    ImGui::Begin("Information"); // Create a window called "Hello,
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui::InputInt("Quality", &settings.quality);
    ImGui::SliderInt("Local size", &settings.local, 1, cl.max);
    ImGui::Text("Global size: %d", settings.global);
    ImGui::End();
    sh->use();
    sh->set("view", cam->view());
    sh->set("projection", projection);
    sh->set("size", .001f);
    sh->set("quality", settings.quality);
    buff->draw();
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

    size_t local[1];
    size_t global[1];
    buff->acquire(cl.queue);
    int err = CL_SUCCESS;
    err |= clSetKernelArg(cl.kernel, 0, sizeof(cl_mem), &buff->mem);
    err |= clSetKernelArg(cl.kernel, 1, sizeof(unsigned int), &settings.amount);
    err |= clSetKernelArg(cl.kernel, 2, sizeof(float), &realPos.x);
    err |= clSetKernelArg(cl.kernel, 3, sizeof(float), &realPos.y);
    err |= clSetKernelArg(cl.kernel, 4, sizeof(float), &deltaTime);
    if (err != CL_SUCCESS) {
      std::cerr << "Error: Failed to set kernel arguments (" << err << ")\n";
      return;
    }
    int n = cl.max / settings.local;
    settings.global = n * settings.local;
    *local = settings.local;
    *global = settings.global;
    err = clEnqueueNDRangeKernel(cl.queue, cl.kernel, 1, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
      std::cerr << "Error: Failed to enqueue kernel (" << err << ")\n";
      return;
    }
    buff->release(cl.queue);
  }

  void cursor_position_callback(double xpos, double ypos) override {
    if (io->WantCaptureMouse)
      return;
    settings.xmouse = xpos;
    settings.ymouse = ypos;
  }

private:
  void setup_compute() {
    cl_uint platforms_amount;
    clGetPlatformIDs(0, nullptr, &platforms_amount);
    std::cout << "Listing platforms (" << platforms_amount << ")...\n";
    cl_platform_id platforms[16];
    clGetPlatformIDs(platforms_amount, platforms, nullptr);
    for (cl_uint i = 0; i < platforms_amount; i++) {
      char buf[1024];
      clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 1024, &buf, nullptr);
      std::cout << "  " << i << ": " << buf << "\n";

    }
    cl_platform_id plat = platforms[0];

    cl_uint devices_amount;
    clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 0, nullptr, &devices_amount);
    std::cout << "Listing devices (" << devices_amount << ")...\n";
    cl_device_id devices[16];
    clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, devices_amount, devices, nullptr);
    for (cl_uint i = 0; i < devices_amount; i++) {
      char buf[1024];
      clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, &buf, nullptr);
      std::cout << "  " << i << ": " << buf << "\n";
    }
    cl.device = devices[0];

#if defined(__APPLE__)
    std::cout << "Using macOS context\n";
    CGLContextObj kCGLContext = CGLGetCurrentContext();
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);

    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)kCGLShareGroup, 0};
#elif defined(_WIN32)
    std::cout << "Using Windows context\n";
    cl_context_properties properties[] = {
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,
        (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)plat,
        0};
#elif defined(__linux__)
    std::cout << "Using Linux context\n";
    cl_context_properties properties[] = {
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties)glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)plat,
        0};
#endif // TODO:  Check support for other platforms
    int err = 0;
    // cl.context = clCreateContext(properties, 0, 0, 0, 0, &err);
    cl.context = clCreateContext(properties, 1, &cl.device, nullptr, nullptr, &err);
    if (!cl.context) {
      std::cerr << "Error: Invalid context (" << err << ")\n";
      return;
    }

    // Create a command queue
    cl.queue = clCreateCommandQueue(cl.context, cl.device, 0, &err);
    if (!cl.queue) {
      std::cerr << "Error: Failed to create a command queue!\n";
      return;
    }

    // Report the device vendor and device name
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};

    err = clGetDeviceInfo(cl.device, CL_DEVICE_VENDOR, sizeof(vendor_name),
                          vendor_name, nullptr);
    err |= clGetDeviceInfo(cl.device, CL_DEVICE_NAME, sizeof(device_name),
                           device_name, nullptr);
    if (err != CL_SUCCESS) {
      std::cerr << "Error: Failed to retrieve device info!\n";
      return;
    }

    std::cout << "Connecting to " << vendor_name << " " << device_name
              << "...\n";

    // compile src code
    std::fstream file(SHADER_PATH "kernel.cl");
    std::stringstream sstring;
    sstring << file.rdbuf();
    std::string source = sstring.str();
    const char *sourcePtr = source.c_str();

    cl.program = clCreateProgramWithSource(
        cl.context, 1, (const char **)&sourcePtr, NULL, &err);
    if (!cl.program || err != CL_SUCCESS) {
      std::cerr << "Error: Failed to create compute program!" << err << "\n";
      return;
    }

    // Build the program executable
    printf("Building compute program...\n");
    err = clBuildProgram(cl.program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
      size_t len;
      char buffer[2048];

      std::cout << "Error: Failed to build CL.program executable!\n";
      clGetProgramBuildInfo(cl.program, cl.device, CL_PROGRAM_BUILD_LOG,
                            sizeof(buffer), buffer, &len);
      std::cout << buffer << "\n";
      return;
    }

    // Create the compute kernel from within the CL.program
    printf("Creating kernel '%s'...\n", "update");
    cl.kernel = clCreateKernel(cl.program, "update", &err);
    if (!cl.kernel || err != CL_SUCCESS) {
      printf("Error: Failed to create compute kernel!\n");
      return;
    }

    // Get the maximum work group size for executing the kernel on the device
    size_t max = 1;
    err = clGetKernelWorkGroupInfo(cl.kernel, cl.device,
                                   CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
                                   &max, NULL);
    if (err != CL_SUCCESS) {
      printf("Error: Failed to retrieve kernel work group info! %d\n", err);
      return;
    }

    cl.max = max;
    printf("Maximum Workgroup Size '%d'\n", cl.max);
  }
};

int main(int argc, char *argv[]) {
  MyApp app;
  app.run();
  return 0;
}

