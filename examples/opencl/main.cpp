#if defined(_WIN32)
#define CL_TARGET_OPENCL_VERSION 120
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#elif defined(__APPLE__) || defined(__MACOSX)
#define CL_TARGET_OPENCL_VERSION 120
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>

#elif defined(__linux__)
#define CL_TARGET_OPENCL_VERSION 120
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glx.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#else
#error "Unsupported platform"
#endif

#include "glcore/camera.h"
#include "glcore/core.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "glm/ext/matrix_clip_space.hpp"

using namespace glm;

#define RANDOM ((float)rand() / (float)(RAND_MAX))

#define CL_CALL(x)                                                                    \
    x;                                                                                \
    if (err != CL_SUCCESS)                                                            \
    {                                                                                 \
        std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << ": " << err << "\n"; \
        exit(EXIT_FAILURE);                                                           \
    }

class MyApp final : public App
{
public:
    MyApp() : App(4, 0, 800, 600, "OpenCL")
    {
        glfwSwapInterval(0);

        findGPUDevice();

        cl_int err;

        // --- 4. Create OpenCL context with OpenGL sharing ---
#if defined(_WIN32)
        cl_context_properties props[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
            CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)cl.platform,
            0};
#elif defined(__APPLE__) || defined(__MACOSX)
        cl_context_properties props[] = {CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()), 0};

#elif defined(__linux__)
        cl_context_properties props[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
            CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)cl.platform,
            0};
#else
#error "Unsupported platform"
#endif
        CL_CALL(cl_context context = clCreateContext(props, 1, &cl.device, nullptr, nullptr, &err))

        // --- 5. Create OpenCL command queue ---
        CL_CALL(cl.queue = clCreateCommandQueue(context, cl.device, 0, &err));

        std::vector<float> data;
        for (int i = 0; i < settings.amount; ++i)
        {
            data.push_back(RANDOM * 2.f - 1.f);
            data.push_back(RANDOM * 2.f - 1.f);
            data.push_back(0.f);
            data.push_back(0.f);
        }

        glGenVertexArrays(1, &gl.vao);
        glGenBuffers(1, &gl.vbo);

        bind();

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // --- 6. Create OpenCL buffer from OpenGL buffer ---
        CL_CALL(cl.buffer = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, gl.vbo, &err));

        // --- 7. Compile kernel ---
        std::fstream file(SHADER_PATH "kernel.cl");
        std::stringstream sstring;
        sstring << file.rdbuf();
        std::string source = sstring.str();
        const char *sourcePtr = source.c_str();
        CL_CALL(cl_program program = clCreateProgramWithSource(context, 1, &sourcePtr, nullptr, &err));
        CL_CALL(err = clBuildProgram(program, 1, &cl.device, nullptr, nullptr, nullptr));
        CL_CALL(cl.kernel = clCreateKernel(program, "update", &err));

        // --- 8. Obtain max work group size ---
        size_t max = 1;
        CL_CALL(err = clGetKernelWorkGroupInfo(cl.kernel, cl.device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &max, nullptr));
        cl.max = max;
    };

    void render() override
    {

        App::render();
        glDisable(GL_DEPTH_TEST);

        ImGui::Begin("Information"); // Create a window called "Hello,
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
        // ImGui::InputInt("Quality", &settings.quality);
        ImGui::SliderFloat("Size", &settings.size, 1.f, 100.f);
        ImGui::SliderInt("Local size", &settings.local, 1, cl.max);
        ImGui::Text("Global size: %d", settings.global);
        ImGui::End();

        glPointSize(settings.size);
        shader.use();
        shader.set("view", camera.view());
        shader.set("projection", projection);
        // shader.set("size", .001f);
        // shader.set("quality", settings.quality);

        bind();
        glDrawArrays(GL_POINTS, 0, settings.amount);
    };

    void update(float deltaTime) override
    {
        const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
        projection = ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);

        float x = (2.0f * settings.xmouse) / width_ - 1.0f;
        float y = 1.0f - (2.0f * settings.ymouse) / height_;

        vec4 pos(x, y, 0.0f, 1.0f);
        mat4 toWorld = inverse(projection * camera.view());
        vec4 realPos = toWorld * pos;

        cl_int err;

        CL_CALL(err = clSetKernelArg(cl.kernel, 0, sizeof(cl_mem), &cl.buffer));
        CL_CALL(err = clSetKernelArg(cl.kernel, 1, sizeof(unsigned int), &settings.amount));
        CL_CALL(err = clSetKernelArg(cl.kernel, 2, sizeof(float), &realPos.x));
        CL_CALL(err = clSetKernelArg(cl.kernel, 3, sizeof(float), &realPos.y));
        CL_CALL(err = clSetKernelArg(cl.kernel, 4, sizeof(float), &deltaTime));

        CL_CALL(err = clEnqueueAcquireGLObjects(cl.queue, 1, &cl.buffer, 0, nullptr, nullptr));

        size_t local[1];
        size_t global[1];
        const size_t n = cl.max / settings.local;
        settings.global = n * settings.local;
        local[0] = settings.local;
        global[0] = settings.global;

        CL_CALL(err = clEnqueueNDRangeKernel(cl.queue, cl.kernel, 1, nullptr, global, local, 0, nullptr, nullptr));
        CL_CALL(err = clEnqueueReleaseGLObjects(cl.queue, 1, &cl.buffer, 0, nullptr, nullptr));
        CL_CALL(err = clFinish(cl.queue));
    }

    void cursor_position_callback(double xpos, double ypos) override
    {
        if (io->WantCaptureMouse)
            return;
        settings.xmouse = xpos;
        settings.ymouse = ypos;
    }

private:
    void bind() const
    {
        glBindVertexArray(gl.vao);
        glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
    }

    void findGPUDevice()
    {
        cl_uint numPlatforms;
        cl_int err;

        // Get the number of platforms
        CL_CALL(err = clGetPlatformIDs(0, nullptr, &numPlatforms));

        std::vector<cl_platform_id> platforms(numPlatforms);
        CL_CALL(err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr));

        for (cl_uint i = 0; i < numPlatforms; ++i)
        {
            char platformName[128];
            CL_CALL(err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platformName), platformName, nullptr));

            std::cout << "Platform " << i + 1 << ": " << platformName << std::endl;

            cl_uint numDevices;
            CL_CALL(err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices))

            std::vector<cl_device_id> devices(numDevices);
            CL_CALL(err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr));

            for (cl_uint j = 0; j < numDevices; ++j)
            {
                char deviceName[128];
                CL_CALL(err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, nullptr));

                cl_device_type deviceType;
                CL_CALL(err = clGetDeviceInfo(devices[j], CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, nullptr));

                std::cout << "  Device " << j + 1 << ": " << deviceName << " (";
                if (deviceType & CL_DEVICE_TYPE_CPU)
                {
                    std::cout << "CPU)";
                }

                else if (deviceType & CL_DEVICE_TYPE_GPU)
                {

                    std::cout << "GPU)";
                    if (cl.device == nullptr)
                    {
                        cl.device = devices[j];
                        cl.platform = platforms[i];
                        std::cout << " <-- Selected";
                    }
                }

                std::cout << std::endl;
            }
        }
    }

    Shader shader = {SHADER_PATH "vert.glsl", SHADER_PATH "frag.glsl"};
    Camera camera = {{0.f, 0.f, 3.f}, 1.f, -90.f};
    mat4 projection = 0;

    struct
    {
        cl_platform_id platform = nullptr;
        cl_device_id device = nullptr;
        cl_kernel kernel = nullptr;
        cl_mem buffer = nullptr;
        cl_command_queue queue = nullptr;
        size_t max = 0;
    } cl;

    struct
    {
        GLuint vbo = 0;
        GLuint vao = 0;
    } gl;

    struct
    {
        float xmouse, ymouse;
        int amount = 1e6;
        int local = 32;
        float size = 1.f;
        size_t global = 0;
    } settings;
};

int main(int argc, char *argv[])
{
    MyApp app;
    app.run();
    return 0;
}
