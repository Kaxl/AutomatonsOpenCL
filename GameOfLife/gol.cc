#define __CL_ENABLE_EXCEPTIONS

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "Display.h"

#define LOCAL_SIZE 32

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
    if (argc != 2) {
        printf("Usage : %s SizeOfGrid\n", argv[0]);
        return 1;
    }
    cl_uint N = (cl_uint)atoi(argv[1]);
    printf("Size of grid : %d\n", N);

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
	std::ifstream sourceFile("gol_kernel.cl");
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));

	// Make program of the source code in the context
	cl::Program program = cl::Program(context, source);

	// Code surounded by try catch in order to get OpenCL errors (eg. compilation error)
	try {
		// Build program for these specific devices
		program.build(devices);

		// Construct kernel
		cl::Kernel k_gol(program,"gol");                // Game Of Life kernel

        // Init of memory
		cl_int* grid = new cl_int[N * N];
		for (cl_uint i = 0; i < (N * N); i++) grid[i] = 0;

        // Init of game of life for test
        grid[N * 3 + 5] = 1;
        grid[N * 3 + 6] = 1;
        grid[N * 3 + 9] = 1;
        grid[N * 3 + 10] = 1;
        grid[N * 3 + 11] = 1;
        grid[N * 2 + 8] = 1;
        grid[N + 6] = 1;

		// Device side memory allocation
		cl::Buffer d_grid = cl::Buffer(context, CL_MEM_READ_WRITE, (N * N) * sizeof(cl_int));
		cl::Buffer d_newGrid = cl::Buffer(context, CL_MEM_READ_WRITE, (N * N) * sizeof(cl_int));

		// Copy host memory into device memory
		queue.enqueueWriteBuffer(d_grid, CL_TRUE, 0, (N * N) * sizeof(cl_int), grid);

		// Set arguments of kernel
		k_gol.setArg(0, N);
		k_gol.setArg(1, d_grid);
		k_gol.setArg(2, d_newGrid);

		// Execute the kernel
		cl::NDRange local(LOCAL_SIZE);
		cl::NDRange global(LOCAL_SIZE);
		std::cout << "Kernel execution" << std::endl;

        Display d(grid, N, N, N);
        d.show();

        for (int i = 0; i < 100; i++) {
		    queue.enqueueNDRangeKernel(k_gol, cl::NullRange, N * N, N * N);

            // Check if we need to use grid or newGrid
            if (i % 2 == 1) {
                // Kernel gol
                k_gol.setArg(1, d_grid);
                k_gol.setArg(2, d_newGrid);
            }
            else {
                // Kernel gol
                k_gol.setArg(1, d_newGrid);
                k_gol.setArg(2, d_grid);
            }
            // Display the array
            std::cout << std::endl;
            std::cout << "--- i=" << i << std::endl;
		    queue.enqueueReadBuffer(d_grid, CL_TRUE, 0, (N * N) * sizeof(cl_int), grid);
            for (cl_uint i = 0; i < N * N; i++) {
                std::cout << grid[i] << " ";
                if (i % N == 0)
                    std::cout << std::endl;
            }

            // Update display
            d.show();
        }

        // Freeze display
        d.waitForKey();

	} catch(cl::Error error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		std::string buildLog;
		program.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildLog);
		std::cerr << buildLog << std::endl;
		exit(1);
	}
}
