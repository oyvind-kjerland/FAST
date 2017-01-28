/*
 * MedianFilter.cpp
 *
 *  Created on: Nov 23, 2016
 *      Author: oyvind
 */

#include "FAST/Algorithms/CoronarySegmentation/MedianFilter.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

MedianFilter::MedianFilter() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/MedianFilter.cl", "MedianFilter");

    // Initialize values
    this->kernelSize = 3;
}

MedianFilter::~MedianFilter() {

}

void MedianFilter::setKernelSize(int size) {
	this->kernelSize = size;
}

void MedianFilter::execute() {
	OpenCLDevice::pointer device = getMainDevice();
	Image::pointer input = getStaticInputData<Image>(0);

	// Width and height
	int width = input->getWidth();
	int height = input->getHeight();
	int depth = input->getDepth();

	// Create new image to contain output
	Image::pointer output = Image::New();
	output->create(width, height, depth, TYPE_FLOAT, 1);

	// Image access
	OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
	OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

	// Program
	cl::Program program = getOpenCLProgram(device, "MedianFilter", "-DVECTORS_16BIT");



	// Split up the work so that it will take less time for each kernel
	// Kernel
	cl::Kernel medianFilterKernel(program, "medianFilter_5x5x5");

	medianFilterKernel.setArg(0, *(inputAccess->get3DImage()));
	medianFilterKernel.setArg(1, *(outputAccess->get()));
	
	for (int d = 0; d < depth; d++) {
		std::cout << "Depth: " << d << std::endl;
		medianFilterKernel.setArg(2, d);
		cl_int err = 0;
		err = device->getCommandQueue().enqueueNDRangeKernel(
				medianFilterKernel,
				cl::NullRange,
				cl::NDRange(width, height, 1),
				cl::NullRange
				);
		device->getCommandQueue().finish();
	}
	
	

	setStaticOutputData<Image>(0, output);

}





