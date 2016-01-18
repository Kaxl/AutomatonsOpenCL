#define __CL_ENABLE_EXCEPTIONS

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
//#include <CL/opencl.h>
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

#include "Display.h"

#define LOCAL_SIZE 32

/**
 * # Game Of Life
 * ------------
 *
 *  0 : Dead cell
 *  1 : Alive cell
 *
 *
 * # Forest Fire
 * -----------
 *
 *  0 : No tree
 *  3 : Healthy tree
 *  2 : Burning tree
 *
 *  Rules
 *  -----
 *
 *  1. A burning tree becomes empty
 *  2. A healthy tree burns if one of his neighbors is burning
 *  3. An empty cell becomes a healthy tree with probability p
 *  4. A healthy tree with no burning neighbors becomes a burning tree with probability f
 *
 *  Initial state : All the grid is fill with healthy trees
 *  Parameters : probabilities p and f
 *
 *  We cannot use rand() function in the kernel so I've decided to pass an array
 *  containing N * N different random values.
 *  Maybe not the best solution, but it will do the trick.
 *
 *
 * # Majorité
 * -----------
 *
 * 0 : vote noir
 * 1 : vote blanc
 *
 */

void initGridGameOfLife(cl_uint* grid, int N) {
    // Gosper glider gun for game of life
    grid[5 * N + 1] = 1;
    grid[6 * N + 1] = 1;
    grid[5 * N + 2] = 1;
    grid[6 * N + 2] = 1;
    grid[5 * N + 11] = 1;
    grid[6 * N + 11] = 1;
    grid[7 * N + 11] = 1;
    grid[4 * N + 12] = 1;
    grid[8 * N + 12] = 1;
    grid[3 * N + 13] = 1;
    grid[9 * N + 13] = 1;
    grid[3 * N + 14] = 1;
    grid[9 * N + 14] = 1;
    grid[6 * N + 15] = 1;
    grid[4 * N + 16] = 1;
    grid[8 * N + 16] = 1;
    grid[5 * N + 17] = 1;
    grid[6 * N + 17] = 1;
    grid[7 * N + 17] = 1;
    grid[6 * N + 18] = 1;
    grid[3 * N + 21] = 1;
    grid[4 * N + 21] = 1;
    grid[5 * N + 21] = 1;
    grid[3 * N + 22] = 1;
    grid[4 * N + 22] = 1;
    grid[5 * N + 22] = 1;
    grid[2 * N + 23] = 1;
    grid[6 * N + 23] = 1;
    grid[1 * N + 25] = 1;
    grid[2 * N + 25] = 1;
    grid[6 * N + 25] = 1;
    grid[7 * N + 25] = 1;
    grid[3 * N + 35] = 1;
    grid[4 * N + 35] = 1;
    grid[3 * N + 36] = 1;
    grid[4 * N + 36] = 1;
    grid[22 * N + 35] = 1;
    grid[23 * N + 35] = 1;
    grid[25 * N + 35] = 1;
    grid[22 * N + 36] = 1;
    grid[23 * N + 36] = 1;
    grid[25 * N + 36] = 1;
    grid[26 * N + 36] = 1;
    grid[27 * N + 36] = 1;
    grid[28 * N + 37] = 1;
    grid[22 * N + 38] = 1;
    grid[23 * N + 38] = 1;
    grid[25 * N + 38] = 1;
    grid[26 * N + 38] = 1;
    grid[27 * N + 38] = 1;
    grid[23 * N + 39] = 1;
    grid[25 * N + 39] = 1;
    grid[23 * N + 40] = 1;
    grid[25 * N + 40] = 1;
    grid[24 * N + 41] = 1;
}

void initGridForestFire(cl_uint* grid, int N) {
    // Fill the game with healthy trees
    for (int i = 0; i < N * N; i++) {
        grid[i] = 3;
    }
}

void initGridMajority(cl_uint* grid, int N) {
    // Initialise la grille pour l automate cellulaire de la majorité
    for (int i = 0; i < N * N; i++) {
        grid[i] = rand()%2;
    }
}

// show available platforms and devices
void showPlatforms(){
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    for (unsigned int i = 0; i < platforms.size(); i++){
        std::cout<<"Platform: "<<platforms[i].getInfo<CL_PLATFORM_NAME>()<<std::endl;
        std::vector<cl::Device> devices;
        platforms[i].getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);
        for (unsigned int j = 0; j < devices.size(); j++) {
            if (devices[j].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU)
                std::cout << "Device: " << " CPU " << " : "<< devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
                    << " compute units " << "( " << devices[j].getInfo<CL_DEVICE_NAME>() << " )" <<std::endl;
            if (devices[j].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
                std::cout << "Device: " << " GPU " << " : "<< devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
                    << " compute units " << "( " << devices[j].getInfo<CL_DEVICE_NAME>() << " )" <<std::endl;
        }
    }
}

// Try to construct a context with the first platform of the requested type
cl::Context getContext(cl_device_type requestedDeviceType, std::vector<cl::Platform>& platforms) {
    // Try to create a context of the requested platform type
    for (unsigned int i = 0; i < platforms.size(); i++){
        try {
            cl_context_properties cps[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[i])(), 0};
            return cl::Context(requestedDeviceType, cps);
        }
        catch(...){ }
    }
    throw CL_DEVICE_NOT_AVAILABLE;
}

int main(int argc, char** argv){
    // Get args from user
    if (argc < 3) {
        std::cout << "Usage : " << argv[0] << " program SizeOfGrid" << std::endl;
        std::cout << "Program : " << std::endl;
        std::cout << "      1 : Game Of Life" << std::endl;
        std::cout << "      2 : Forest Fire" << std::endl;
        std::cout << "      3 : Majority" << std::endl;
        return 1;
    }
    int algo = atoi(argv[1]);
    std::cout << "Algo : " << algo << std::endl;
    cl_double p = 0.3;
    cl_double f = 0.00006;

    if (algo == 2) {
        if (argc != 5) {
            std::cout << "Usage : " << argv[0] << " program SizeOfGrid probGrow probFire" << std::endl;
        }
        else {
            p = (cl_double)atof(argv[3]);
            f = (cl_double)atof(argv[4]);
            std::cout << "Probability of growth : " << p << std::endl;
            std::cout << "Probability of fire : " << f << std::endl;
        }
    }
    cl_uint N = (cl_uint)atoi(argv[2]);
    std::cout << "Size of grid : " << N << std::endl;

    showPlatforms();
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    // Get a context on a platform containing at least one GPU
    cl::Context context = getContext(CL_DEVICE_TYPE_GPU, platforms);

    // Get a list of devices on this platform
    std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

    // Create a command queue and use the first device of the platform
    cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
    std::string deviceName;
    devices[0].getInfo(CL_DEVICE_NAME, &deviceName);
    std::cout << "Command queue created successfuly, "
              << "Kernel will be executed on : " << deviceName << std::endl;

    // Read source file
    const char* kernelFile = "gameoflife_kernel.cl";
    switch (algo) {
        case 1:     // Game Of Life
            kernelFile = "gameoflife_kernel.cl";
            break;
        case 2:     // Forest Fire
            kernelFile = "forestfire_kernel.cl";
            break;
        case 3:		// Majorité
        	kernelFile = "majority_kernel.cl";
            break;
        default:
            kernelFile = "gameoflife_kernel.cl";
            break;
    }
    std::cout << "Kernel file used : " << kernelFile << std::endl;

    std::ifstream sourceFile(kernelFile);
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
    cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

    // Make program of the source code in the context
    cl::Program program = cl::Program(context, source);

    // Code surounded by try catch in order to get OpenCL errors (eg. compilation error)
    try {
        // Build program for these specific devices
        program.build(devices);


        // Set the kernel to use
        const char* kernelProgram = "gameoflife";
        switch (algo) {
            case 1:     // Game Of Life
                kernelProgram = "gameoflife";
                break;
            case 2:     // Forest Fire
                kernelProgram = "forestfire";
                break;
            case 3:		// Majorité
            	kernelProgram = "majority";
                break;
            default:
                kernelFile = "gameoflife";
                break;
        }
        std::cout << "Kernel Program used : " << kernelProgram << std::endl;
        // Construct kernel
        cl::Kernel k_automaton(program, kernelProgram);

        // Init of memory
        cl_uint* grid = new cl_uint[N * N];
        for (cl_uint i = 0; i < (N * N); i++) grid[i] = 0;
        // Random values
        cl_double* randoms;

        // Init of grid
        switch (algo) {
            case 1:     // Game Of Life
                initGridGameOfLife(grid, N);
                break;
            case 2:     // Forest Fire
                initGridForestFire(grid, N);
                break;
            case 3:		// Majorité
            	initGridMajority(grid, N);
                break;
            default:
                initGridGameOfLife(grid, N);
                break;
        }

        // Device side memory allocation
        cl::Buffer d_grid = cl::Buffer(context, CL_MEM_READ_WRITE, (N * N) * sizeof(cl_int));
        cl::Buffer d_newGrid = cl::Buffer(context, CL_MEM_READ_WRITE, (N * N) * sizeof(cl_int));
        // Declaration of arguments of Forest Fire needs to be declared outside a if statement
        cl::Buffer d_p;
        cl::Buffer d_f;
        cl::Buffer d_randoms;   // Random values

        // Copy host memory into device memory
        queue.enqueueWriteBuffer(d_grid, CL_TRUE, 0, (N * N) * sizeof(cl_int), grid);

        // Set arguments of kernel
        k_automaton.setArg(0, N);
        k_automaton.setArg(1, d_grid);
        k_automaton.setArg(2, d_newGrid);

        // Forest Fire : add probabilities into arguments
        if (algo == 2) {
            // Growth probability
            k_automaton.setArg(3, p);
            // Fire probability
            k_automaton.setArg(4, f);
            // Random values for each id
            randoms = new cl_double[N * N];
            srand(time(NULL));
            // Fill the array with random value, one for each id
            for (cl_uint i = 0; i < (N * N); i++) randoms[i] = ((cl_double)rand() / (cl_double)RAND_MAX);
            d_randoms = cl::Buffer(context, CL_MEM_READ_WRITE, N * N * sizeof(cl_double));
            queue.enqueueWriteBuffer(d_randoms, CL_TRUE, 0, N * N * sizeof(cl_double), randoms);
            k_automaton.setArg(5, d_randoms);
        }

        // Execute the kernel
        std::cout << "Kernel execution" << std::endl;

        Display d(grid, N, N, N);
        d.show();

        for (int i = 0; i < 10000; i++) {
            queue.enqueueNDRangeKernel(k_automaton,
                                       cl::NullRange,       // Start offset
                                       cl::NDRange(N * N),  // Nb of work-items
                                       cl::NDRange(1));     // Nb of work-items by work-group

            // Check if we need to use grid or newGrid
            if (i % 2 == 1) {
                k_automaton.setArg(1, d_grid);
                k_automaton.setArg(2, d_newGrid);
            }
            else {
                k_automaton.setArg(1, d_newGrid);
                k_automaton.setArg(2, d_grid);
            }

            // Get the values
            queue.enqueueReadBuffer(d_grid, CL_TRUE, 0, (N * N) * sizeof(cl_int), grid);
            // Display the array
            std::cout << std::endl;
            std::cout << "--- i=" << i << std::endl;
            //for (cl_uint i = 0; i < N * N; i++) {
            //    std::cout << grid[i] << " ";
            //    if (i % N == 0)
            //        std::cout << std::endl;
            //}

            // Generates new random values for Forest Fire
            if (algo == 2) {
                for (cl_uint i = 0; i < (N * N); i++) randoms[i] = ((cl_double)rand() / (cl_double)RAND_MAX);
                d_randoms = cl::Buffer(context, CL_MEM_READ_ONLY, N * N * sizeof(cl_double));
                queue.enqueueWriteBuffer(d_randoms, CL_TRUE, 0, N * N * sizeof(cl_double), randoms);
                k_automaton.setArg(5, d_randoms);
            }

            // Update display
            d.show();
        }

        // Freeze display
        d.waitForKey();

        delete[] grid;

	} catch(cl::Error error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		std::string buildLog;
		program.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildLog);
		std::cerr << buildLog << std::endl;
		exit(1);
	}
}
