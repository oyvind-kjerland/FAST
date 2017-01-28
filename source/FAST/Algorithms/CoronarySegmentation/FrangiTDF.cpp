#include "FAST/Algorithms/CoronarySegmentation/FrangiTDF.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

FrangiTDF::FrangiTDF() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/FrangiTDF.cl");

}

void FrangiTDF::execute() {
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);

    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;

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

        kernel = cl::Kernel(program, "FrangiTDF3D");
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
    }
}

void FrangiTDF::setTubeConstants(float alpha, float beta, float gamma) {
	this->alpha = alpha;
	this->beta = beta;
	this->gamma = gamma;
}


}


