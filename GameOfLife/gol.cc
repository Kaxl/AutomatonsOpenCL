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


// show available platforms and devices
void showPlatforms(){
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	for (unsigned int i=0;i<platforms.size();i++){
		std::cout<<"Platform: "<<platforms[i].getInfo<CL_PLATFORM_NAME>()<<std::endl;
		std::vector<cl::Device> devices;
		platforms[i].getDevices(CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU, &devices);
		for (unsigned int j=0;j<devices.size();j++) {
			if (devices[j].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU)
            std::cout << "Device: " << " CPU " << " : "<< devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
                      << " compute units " << "( " << devices[j].getInfo<CL_DEVICE_NAME>() << " )" <<std::endl;
			if (devices[j].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
            std::cout << "Device: " << " GPU " << " : "<< devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>()
                      << " compute units " << "( " << devices[j].getInfo<CL_DEVICE_NAME>() << " )" <<std::endl;
		}
	}
}

// try to construct a context with the first platform of the requested type
cl::Context getContext(cl_device_type requestedDeviceType, std::vector<cl::Platform>& platforms){
	// try to create a context of the requested platform type
	for (unsigned int i=0;i<platforms.size();i++){
		try {
			cl_context_properties cps[] = {CL_CONTEXT_PLATFORM,(cl_context_properties)(platforms[i])(),0};
			return cl::Context(requestedDeviceType,cps);
		}
		catch(...){ }
	}
	throw CL_DEVICE_NOT_AVAILABLE;
}

int main(int argc, char** argv){
	showPlatforms();
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	// get a context on a platform containing at least one GPU
	cl::Context context = getContext(CL_DEVICE_TYPE_GPU, platforms);

	// get a list of devices on this platform
	std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

	// create a command queue and use the first device of the platform
	cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);
	std::string deviceName;
	devices[0].getInfo(CL_DEVICE_NAME, &deviceName);
	std::cout << "Command queue created successfuly, "
             << "kernels will be executed on : " << deviceName << std::endl;

	// read source file
	std::ifstream sourceFile("gol_kernels.cl");
	std::string sourceCode(std::istreambuf_iterator<char>(sourceFile),(std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1,std::make_pair(sourceCode.c_str(),sourceCode.length()+1));

	// make program of the source code in the context
	cl::Program program = cl::Program(context,source);
	// code surounded by try catch in order to get OpenCL errors (eg. compilation error)
	try {
		// build program for these specific devices
		program.build(devices);

		// construct kernel
		cl::Kernel k_gol(program,"GOL");
		cl::Kernel k_ghostCols(program,"ghostCols");
		cl::Kernel k_ghostRows(program,"ghostRows");

		// host side memory allocation and initialization
		cl_uint  N = 100;
		switch(argc) {
	      case 2: N = (cl_uint)atoi(argv[1]);
	      case 1: break;
	      default: std::cout << "usage error" << std::endl;
	    }

		cl_int* grid = new cl_int[N * N];
		for (cl_uint i = 0; i < (N * N); i++) grid[i] = 0;

        grid[N / 2 + N * N / 2] = 1;
        grid[N / 2 + N * N / 2 + 1] = 1;
        grid[N / 2 + N * N / 2 + 4] = 1;
        grid[N / 2 + N * N / 2 + 5] = 1;
        grid[N / 2 + N * N / 2 + 6] = 1;
        grid[N / 2 + N * (N / 2 - 1) + 3] = 1;
        grid[N / 2 + N * (N / 2 - 2) + 1] = 1;

		// device side memory allocation
		cl::Buffer d_grid = cl::Buffer(context, CL_MEM_READ_WRITE, (N * N) * sizeof(cl_int));
		cl::Buffer d_newGrid = cl::Buffer(context, CL_MEM_READ_WRITE, (N * N) * sizeof(cl_int));

		// copy host memory into device memory
		queue.enqueueWriteBuffer(d_grid, CL_TRUE, 0, (N * N) * sizeof(cl_int), grid);

		// set arguments to kernel
		k_gol.setArg(0, N * N);
		k_gol.setArg(1, d_grid);
		k_gol.setArg(2, d_newGrid);

		// execute the kernel
		cl::NDRange local(N);
		cl::NDRange global(N);
		std::cout << "Kernel execution" << std::endl;
		//queue.enqueueNDRangeKernel(k_ghostRows,cl::NullRange,global,local);
		//queue.enqueueNDRangeKernel(k_ghostCols,cl::NullRange,global,local);

        Display d(grid, (unsigned int)N, (unsigned int)N, (unsigned int)N);
        d.show();

        for (int i = 0; i < 10; i++) {
		    //queue.enqueueNDRangeKernel(k_ghostRows, cl::NullRange, global, local);
		    //queue.enqueueNDRangeKernel(k_ghostCols, cl::NullRange, global, local);
		    queue.enqueueNDRangeKernel(k_gol, cl::NullRange, global, local);

            // Check if we need to use grid or newGrid
            if (i % 2 == 1) {
                // Kernel GOL
                k_gol.setArg(1, d_grid);
                k_gol.setArg(2, d_newGrid);
                // Kernel Ghost Rows
                // Kernel Ghost Cols
            }
            else {
                // Kernel GOL
                k_gol.setArg(1, d_newGrid);
                k_gol.setArg(2, d_grid);
                // Kernel Ghost Rows
                // Kernel Ghost Cols
            }
            std::cout << "--- i=" << i << std::endl;
		    queue.enqueueReadBuffer(d_grid,CL_TRUE,0,(N * N) * sizeof(cl_int), grid);
            for (cl_uint i = 1; i <= N * N; i++) {
                std::cout << grid[i] << ",";
                if (i % N == 0) {
                    std::cout << "END" << std::endl;
                }
            }

            // Update display
            d.show();
        }

		// getting the result
		queue.enqueueReadBuffer(d_grid,CL_TRUE,0,(N * N) * sizeof(cl_int), grid);
        for (cl_uint i = 1; i <= N * N; i++) {
            std::cout << grid[i] << ",";
            if (i % N == 0) {
                std::cout << "END" << std::endl;
            }
        }

        // Freeze display
        d.waitForKey();

	} catch(cl::Error error) {
		std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
		std::string buildLog;
		program.getBuildInfo(devices[0],CL_PROGRAM_BUILD_LOG,&buildLog);
		std::cerr << buildLog << std::endl;
		exit(1);
	}
}
