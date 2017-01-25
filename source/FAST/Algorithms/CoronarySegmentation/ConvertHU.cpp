/*
 * ConvertHU.cpp
 *
 *  Created on: Oct 29, 2016
 *      Author: oyvind
 */

#include "FAST/Algorithms/CoronarySegmentation/ConvertHU.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

ConvertHU::ConvertHU() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/CoronarySegmentation/ConvertHU.cl", "ConvertHU");
    maxHU = 500;
    minHU = 0;
}

ConvertHU::~ConvertHU() {

}


void ConvertHU::setMaxHU(float maxHU) {
	this->maxHU = maxHU;
}

void ConvertHU::setMinHU(float minHU) {
	this->minHU = minHU;
}

void ConvertHU::execute() {
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
	cl::Program program = getOpenCLProgram(device, "ConvertHU", "-DVECTORS_16BIT");

	// Kernel
	cl::Kernel convertHUKernel(program, "convertHU_3D");

	convertHUKernel.setArg(0, *(inputAccess->get3DImage()));
	convertHUKernel.setArg(1, *(outputAccess->get()));
	convertHUKernel.setArg(2, minHU);
	convertHUKernel.setArg(3, maxHU);

	device->getCommandQueue().enqueueNDRangeKernel(
			convertHUKernel,
			cl::NullRange,
			cl::NDRange(width, height, depth),
			cl::NullRange
			);

	device->getCommandQueue().finish();

	setStaticOutputData<Image>(0, output);

}
