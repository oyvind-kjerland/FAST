#include "FAST/Algorithms/CoronarySegmentation/PrepareRidge.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

PrepareRidge::PrepareRidge() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(1, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/PrepareRidge.cl");

}

void PrepareRidge::execute() {
	std::cout << "Execute PrepareRidge" << std::endl;

	// Get TDF input
	Image::pointer input = getStaticInputData<Image>(0);

	// Create candidates
	Image::pointer candidates = getStaticOutputData<Image>(0);
	Image::pointer neighbors = getStaticOutputData<Image>(0);



    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;

    Image::pointer output = Image::New();
    output->create(
			input->getWidth(),
			input->getHeight(),
			input->getDepth(),
			type,
			1
	);


    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

        kernel = cl::Kernel(program, "PrepareRidge3D");
        kernel.setArg(0, *inputAccess->get3DImage());
        kernel.setArg(1, *outputAccess->get());
		kernel.setArg(2, alpha);
		kernel.setArg(3, beta);
		kernel.setArg(4, gamma);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
        );
        device->getCommandQueue().finish();
    }



    setStaticOutputData<Image>(0, output);

    std::cout << "Finish Execute PrepareRidge" << std::endl;
}

void PrepareRidge::setTubeConstants(float alpha, float beta, float gamma) {
	this->alpha = alpha;
	this->beta = beta;
	this->gamma = gamma;
}


}


