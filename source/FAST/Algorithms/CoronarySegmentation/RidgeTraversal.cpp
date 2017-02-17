#include "FAST/Algorithms/CoronarySegmentation/RidgeTraversal.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

RidgeTraversal::RidgeTraversal() {
    createInputPort<Image>(RIDGE_TRAVERSAL_INPUT_TDF);
    createInputPort<Image>(RIDGE_TRAVERSAL_INPUT_TANGENTS);
    createInputPort<Image>(RIDGE_TRAVERSAL_INPUT_CANDIDATES);
    createOutputPort<Image>(RIDGE_TRAVERSAL_OUTPUT_NEIGHBORS, OUTPUT_DEPENDS_ON_INPUT, RIDGE_TRAVERSAL_INPUT_TDF);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/RidgeTraversal.cl");

}

void RidgeTraversal::execute() {
	std::cout << "Execute RidgeTraversal" << std::endl;

	// Get TDF input
	Image::pointer tdf = getStaticInputData<Image>(RIDGE_TRAVERSAL_INPUT_TDF);

	// Get tangent input
	Image::pointer tangents = getStaticInputData<Image>(RIDGE_TRAVERSAL_INPUT_TANGENTS);

	// Get ridge candidates
	Image::pointer candidates = getStaticInputData<Image>(RIDGE_TRAVERSAL_INPUT_CANDIDATES);

	// Create neighbors
	Image::pointer neighbors = getStaticOutputData<Image>(RIDGE_TRAVERSAL_OUTPUT_NEIGHBORS);


	int width = tdf->getWidth();
	int height = tdf->getHeight();
	int depth = tdf->getDepth();

    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;

    neighbors->create(
			width,
			height,
			depth,
			type,
			2
	);


    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;


        OpenCLImageAccess::pointer tdfAccess = tdf->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer tangentsAccess = tangents->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer neighborsAccess = neighbors->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

        kernel = cl::Kernel(program, "FindNeighbors");
        kernel.setArg(0, *tdfAccess->get3DImage());
        kernel.setArg(1, *tangentsAccess->get3DImage());
		kernel.setArg(2, *neighborsAccess->get());
		kernel.setArg(3, t_low);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width, height, depth),
                cl::NullRange
        );
        device->getCommandQueue().finish();
    }



    std::cout << "Finish Execute RidgeTraversal" << std::endl;
}


void RidgeTraversal::setTLow(float t_low) {
	this->t_low = t_low;
}

void RidgeTraversal::setTDFInputConnection(ProcessObjectPort port) {
	return setInputConnection(RIDGE_TRAVERSAL_INPUT_TDF, port);
}

void RidgeTraversal::setTangentsInputConnection(ProcessObjectPort port) {
	return setInputConnection(RIDGE_TRAVERSAL_INPUT_TANGENTS, port);
}

void RidgeTraversal::setRidgeCandidatesInputConnection(ProcessObjectPort port) {
	return setInputConnection(RIDGE_TRAVERSAL_INPUT_CANDIDATES, port);
}

ProcessObjectPort RidgeTraversal::getNeighborsOutputPort() {
	return getOutputPort(RIDGE_TRAVERSAL_OUTPUT_NEIGHBORS);
}

}


