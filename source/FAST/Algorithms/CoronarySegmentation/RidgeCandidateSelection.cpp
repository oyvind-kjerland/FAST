#include "FAST/Algorithms/CoronarySegmentation/RidgeCandidateSelection.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

RidgeCandidateSelection::RidgeCandidateSelection() {
    std::cout << "heyoo" << std::endl;
	// Input is a gradient volume
	createInputPort<Image>(0);

	// Output 0 is a volume containing the candidates
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

	// Output 1 is a volume containing the neighbor index
	createOutputPort<Image>(1, OUTPUT_DEPENDS_ON_INPUT, 0);

	// Create OpenCL program
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/RidgeCandidateSelection.cl");

    t_high = 0;
    t_low = 0;
}

void RidgeCandidateSelection::execute() {
    DataType type = TYPE_FLOAT;
	std::cout << "execute RidgeCandidateSelection" << std::endl;
	Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer candidates = getStaticOutputData<Image>(0);
	Image::pointer neighbors = getStaticOutputData<Image>(1);

	int width = input->getWidth();
	int height = input->getHeight();
	int depth = input->getDepth();

	float t_max = input->calculateMaximumIntensity();

	// Create candidates volumeRidgeCandidateSelection
	candidates->create(width, height, depth, type, 1);

	// Create neighbors volume
	neighbors->create(width, height, depth, type, 1);

    std::string buildOptions = "";


    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;
        
        
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
		OpenCLBufferAccess::pointer candidatesAccess = candidates->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		OpenCLBufferAccess::pointer neighborsAccess = neighbors->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

		kernel = cl::Kernel(program, "RidgeCandidateSelection");


		kernel.setArg(0, *inputAccess->get3DImage());
		kernel.setArg(1, *candidatesAccess->get());
		kernel.setArg(2, *neighborsAccess->get());
		kernel.setArg(3, t_high);
		kernel.setArg(4, t_low);
		kernel.setArg(5, t_max);
			
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
        );
        device->getCommandQueue().finish();

    }
}

ProcessObjectPort RidgeCandidateSelection::getCandidatesOuptutPort() {
	return getOutputPort(0);
}

ProcessObjectPort RidgeCandidateSelection::getNeighborsOutputPort() {
	return getOutputPort(1);
}

void RidgeCandidateSelection::setTHigh(float t_high) {
	this->t_high = t_high;
}

void RidgeCandidateSelection::setTLow(float t_low) {
	this->t_low = t_low;
}


}



