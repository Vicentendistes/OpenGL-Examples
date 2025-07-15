#include "game_of_life_opencl.hpp"

// Definiciones de los miembros estáticos:
bool             GameOfLifeOpenCL::clInitialized = false;
cl_platform_id   GameOfLifeOpenCL::platform      = nullptr;
cl_device_id     GameOfLifeOpenCL::device        = nullptr;
cl_context       GameOfLifeOpenCL::context       = nullptr;
cl_command_queue GameOfLifeOpenCL::queue         = nullptr;
cl_program       GameOfLifeOpenCL::program       = nullptr;
cl_kernel        GameOfLifeOpenCL::kernel        = nullptr;

using ubyte = unsigned char;

#define CL_CALL(x)                                                                    \
    x;                                                                                \
    if (err != CL_SUCCESS)                                                            \
    {                                                                                 \
        std::cerr << "Error: " << __FILE__ << ":" << __LINE__ << ": " << err << "\n"; \
        exit(EXIT_FAILURE);                                                           \
    }

cl_int err;


GameOfLifeOpenCL::GameOfLifeOpenCL(uint rows, uint cols)
: rows_(rows)
, cols_(cols)
, worldSize_(rows_ * cols_)
{
    // 1) Inicializa context/queue si es la primera instancia
    initializeOpenCL();

    // 2) Carga y compila tu kernel .cl → programa + kernel
    buildProgram();

    // 3) Reserva buffers GPU ↔ CPU
    setupBuffers();

    // 4) Calcula tamaños local/global para NDRange
    calculateWorkSizes();
}

void GameOfLifeOpenCL::initializeOpenCL() {
    if (clInitialized) return;

    // 1. Enumerar plataformas
    cl_uint numPlat;
    CL_CALL(err = clGetPlatformIDs(0, nullptr, &numPlat));
    std::vector<cl_platform_id> plats(numPlat);
    CL_CALL(err = clGetPlatformIDs(numPlat, plats.data(), nullptr));
    platform = plats[0];

    // 2. Enumerar dispositivos GPU en esa plataforma
    cl_uint numDev;
    CL_CALL(err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDev));
    std::vector<cl_device_id> devs(numDev);
    CL_CALL(err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDev, devs.data(), nullptr));
    device = devs[0];

    // 3. Preparar propiedades para compartir con OpenGL
    cl_context_properties props[] = {
    #if defined(_WIN32)
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,   (cl_context_properties)wglGetCurrentDC(),
    #elif defined(__APPLE__)
        CL_CGL_SHAREGROUP_KHR, (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
    #elif defined(__linux__)
        CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,(cl_context_properties)glXGetCurrentDisplay(),
    #endif
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0
    };

    // 4. Crear contexto compartido GL↔CL
    CL_CALL(context = clCreateContext(props, 1, &device, nullptr, nullptr, &err););

    // 5. Crear cola de comandos   
    CL_CALL(queue = clCreateCommandQueue(context, device, 0, &err));
    
    clInitialized = true;
}

void GameOfLifeOpenCL::buildProgram() {
    // Leer código fuente
    std::string src = loadKernelSource(KERNEL_FILE);
    const char* sourcePtr = src.c_str();
    // Crear y compilar programa desde fuente
    CL_CALL(program = clCreateProgramWithSource(context, 1, &sourcePtr, nullptr, &err));
    CL_CALL(err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr));
    // Extraer kernel de función 'update'
    CL_CALL(kernel = clCreateKernel(program, "life", &err));
}

std::string GameOfLifeOpenCL::loadKernelSource(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el kernel: " + std::string(path));
    }
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

void GameOfLifeOpenCL::setupBuffers() {
    // Prepara la copia en host
    simCurrHost.assign(worldSize_, 0);
    simNextHost.assign(worldSize_, 0);

    // Reserva device buffers con clCreateBuffer
    cl_int err;
    size_t bytes = worldSize_ * sizeof(ubyte);

    CL_CALL(simCurr = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, nullptr, &err));
    CL_CALL(simNext = clCreateBuffer(context, CL_MEM_READ_WRITE, bytes, nullptr, &err));
}

void GameOfLifeOpenCL::attachStateVBO(GLuint stateVBO) {
    // Creamos el cl_mem compartido:
    CL_CALL(
        glStateBuffer = clCreateFromGLBuffer(
            context,
            CL_MEM_WRITE_ONLY,     // OpenCL solo va a escribir en él
            stateVBO,
            &err
        )
    );
}

void GameOfLifeOpenCL::calculateWorkSizes() {
    // Número total de celdas procesadas por work-item
    const size_t cellsPerItem = blockSize * cells_per_thread;
    // Número de work-groups necesarios (redondeo hacia arriba)
    blocks = (worldSize_ + cellsPerItem - 1) / cellsPerItem;

    // Tamaño de work-group (número de work-items por grupo)
    localSize_ = blockSize;
    // Tamaño global total (múltiplo de localSize_)
    globalSize_ = blocks * localSize_;
}

void GameOfLifeOpenCL::initialize() {
    std::fill(simCurrHost.begin(), simCurrHost.end(), 0);
    std::fill(simNextHost.begin(), simNextHost.end(), 0);

    size_t bytes = worldSize_ * sizeof(ubyte);

    uploadGrid();
}

void GameOfLifeOpenCL::initializeRandom() {
    std::mt19937 gen(123);
    std::uniform_int_distribution<> d(0,1);
    for (size_t i = 0; i < worldSize_; ++i) {
        simCurrHost[i] = (ubyte)d(gen);
        simNextHost[i] = simCurrHost[i];
    }
    uploadGrid();
}

void GameOfLifeOpenCL::setCell(uint i, uint j, ubyte state) {
    size_t idx = j * cols_ + i;
    simCurrHost[idx] = state;
    uploadGrid();
}

void GameOfLifeOpenCL::uploadGrid() {
    size_t bytes = worldSize_ * sizeof(ubyte);

    // subir grid.host -> grid.dev
    CL_CALL(
      err = clEnqueueWriteBuffer(queue, simCurr, CL_FALSE, 0, bytes, simCurrHost.data(), 0, nullptr, nullptr)
    );
    // adquirir el buffer
    CL_CALL(err = clEnqueueAcquireGLObjects(
        queue, 1, &glStateBuffer, 0, nullptr, nullptr));
    //    luego la copia
    CL_CALL(err = clEnqueueCopyBuffer(
        queue,
        simCurr,
        glStateBuffer,
        0, 0,
        worldSize_ * sizeof(ubyte),
        0, nullptr, nullptr));
    //    y por último ‘liberar’ el buffer GL
    CL_CALL(err = clEnqueueReleaseGLObjects(
        queue, 1, &glStateBuffer, 0, nullptr, nullptr));
    CL_CALL(err = clFinish(queue));
}

void GameOfLifeOpenCL::copyGridToHost() {
    CL_CALL(err = clEnqueueReadBuffer(queue, simCurr, CL_TRUE, 0, worldSize_ * sizeof(ubyte), simCurrHost.data(), 0, nullptr, nullptr));
}

void GameOfLifeOpenCL::step() {
    // 1) Fija argumentos
    cl_uint arg = 0;
    CL_CALL(err = clSetKernelArg(kernel, arg++, sizeof(cl_mem), &simCurr));
    CL_CALL(err = clSetKernelArg(kernel, arg++, sizeof(cl_mem), &simNext));
    CL_CALL(err = clSetKernelArg(kernel, arg++, sizeof(cl_uint), &cols_));
    CL_CALL(err = clSetKernelArg(kernel, arg++, sizeof(cl_uint), &rows_));
    CL_CALL(err = clSetKernelArg(kernel, arg++, sizeof(cl_uint), &cells_per_thread));

    // 2) Encola y ejecuta
    CL_CALL(err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize_, &localSize_, 0, nullptr, nullptr));

    // 3) Asegura que termine antes de swap
    CL_CALL(err = clFinish(queue));

    // 4) Intercambia buffers para la próxima iteración
    std::swap(simCurr, simNext);

    // 5) copiar simCurr → glStateBuffer (todo en GPU)
    //    primero hay que ‘adquirir’ el buffer GL
    CL_CALL(err = clEnqueueAcquireGLObjects(
        queue, 1, &glStateBuffer, 0, nullptr, nullptr));
    //    luego la copia
    CL_CALL(err = clEnqueueCopyBuffer(
        queue,
        simCurr,
        glStateBuffer,
        0, 0,
        worldSize_ * sizeof(ubyte),
        0, nullptr, nullptr));
    //    y por último ‘liberar’ el buffer GL
    CL_CALL(err = clEnqueueReleaseGLObjects(
        queue, 1, &glStateBuffer, 0, nullptr, nullptr));
    CL_CALL(err = clFinish(queue));
}

ubyte* GameOfLifeOpenCL::getGrid() {
    copyGridToHost();
    return const_cast<ubyte*>(simCurrHost.data());
}

void GameOfLifeOpenCL::setGrid(const ubyte* grid) {
  std::copy(grid, grid + worldSize_, simCurrHost.begin());
  uploadGrid();
}

GameOfLifeOpenCL::~GameOfLifeOpenCL() {
    // Liberar recursos OpenCL si es necesario
    if (glStateBuffer) clReleaseMemObject(glStateBuffer);
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
    if (simCurr) clReleaseMemObject(simCurr);
    if (simNext) clReleaseMemObject(simNext);
    clInitialized = false;
}