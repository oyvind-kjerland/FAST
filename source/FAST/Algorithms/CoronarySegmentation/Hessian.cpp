#include "FAST/Algorithms/CoronarySegmentation/Hessian.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

Hessian::Hessian() {
    
	// Input is a gradient volume
	createInputPort<Image>(0);

	// Output 0 is a volume containing eigenvalues where each element is a 3d vector (h1,h2,h3)
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);

	// Output 1 is a volume containing the tangents, used for ridge traversal, each element corresponds to the smallest eigenvector of the hessian matrix
	createOutputPort<Image>(1, OUTPUT_DEPENDS_ON_INPUT, 0);

	// Create OpenCL program
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/Hessian.cl");
}

void Hessian::execute() {
    DataType type = TYPE_FLOAT;
	std::cout << "execute Hessian" << std::endl;
	Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer eigenvalues = getStaticOutputData<Image>(0);
	Image::pointer tangents = getStaticOutputData<Image>(1);

	int width = input->getWidth();
	int height = input->getHeight();
	int depth = input->getDepth();

	// Create eigenvalue volume
	eigenvalues->create(width, height, depth, type, 3);

	// Create tangents volume
	tangents->create(width, height, depth, type, 3);

    std::string buildOptions = "";


    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        
		kernel = cl::Kernel(program, "Hessian");
        
		// input
		kernel.setArg(0, *inputAccess->get3DImage());

		// eigenvalues
		OpenCLBufferAccess::pointer eigenvaluesAccess = eigenvalues->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(1, *eigenvaluesAccess->get());

		// Tangents
		OpenCLBufferAccess::pointer tangentsAccess = tangents->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel.setArg(2, *tangentsAccess->get());
			
        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
        );
        device->getCommandQueue().finish();

    }
}

ProcessObjectPort Hessian::getEigenvaluesOuptutPort() {
	return getOutputPort(0);
}

ProcessObjectPort Hessian::getTangentsOutputPort() {
	return getOutputPort(1);
}

}


