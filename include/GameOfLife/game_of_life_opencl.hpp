#ifndef GAME_OF_LIFE_OPENCL_HPP
#define GAME_OF_LIFE_OPENCL_HPP
#include "game_of_life.hpp"

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GLFW/glfw3.h>
#ifdef __linux__
#  include <GL/glx.h>
#endif
#ifdef _WIN32
#  include <windows.h>
#endif
#ifdef __APPLE__
#  include <OpenGL/OpenGL.h>
#endif

#include <vector>
#include <random>
#include <stdexcept>

#include <iostream>
#include <fstream>
#include <sstream>

using ubyte = uint8_t;
using uint  = unsigned int;

class GameOfLifeOpenCL : public GameOfLife {
private:
    static bool clInitialized;
    static cl_context context;
    static cl_platform_id platform;
    static cl_device_id device;
    static cl_command_queue queue;
    static cl_program program;
    static cl_kernel kernel;

    uint rows_, cols_;
    size_t worldSize_;
    
    // Buffers separados para CPU y GPU
    std::vector<ubyte> simCurrHost;
    std::vector<ubyte> simNextHost;

    // Buffers puros de simulación OpenCL (ping-pong)
    cl_mem simCurr;
    cl_mem simNext;
    // Buffer compartido con OpenGL para estado de instancia
    cl_mem glStateBuffer;

    // Gestión de bloques para NDRange
    size_t blocks;
    size_t blockSize = 32 * 32;
    uint cells_per_thread = static_cast<unsigned int>(sqrt(blockSize));
    // Tamaños de NDRange calculados
    size_t localSize_;
    size_t globalSize_; 
    
     // Helpers
    static void initializeOpenCL();
    void buildProgram();
    void setupBuffers();
    void calculateWorkSizes();
    void uploadGrid();
    std::string loadKernelSource(const char* path);

public:
    GameOfLifeOpenCL(uint rows, uint cols);
    ~GameOfLifeOpenCL() override;

    void initialize() override;
    void initializeRandom() override;
    void step() override;
    void attachStateVBO(GLuint stateVBO);

    void setCell(uint i, uint j, ubyte state) override;
    
    ubyte* getGrid() override;
    void setGrid(const ubyte* grid) override;
    void copyGridToHost() override;
};
#endif // GAME_OF_LIFE_OPENCL_HPP