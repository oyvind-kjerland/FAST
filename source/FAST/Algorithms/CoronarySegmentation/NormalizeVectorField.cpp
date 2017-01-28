#include "FAST/Algorithms/CoronarySegmentation/NormalizeVectorField.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

NormalizeVectorField::NormalizeVectorField() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/NormalizeVectorField.cl");

}

void NormalizeVectorField::execute() {
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);

    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;

	 output->create(
			input->getWidth(),
			input->getHeight(),
			input->getDepth(),
			type,
			3
	);
    

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;

        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

		// Check if the vector field should be normalized within a threshold
		if (useMaxLength) {
			kernel = cl::Kernel(program, "NormalizeVectorFieldMax");
			kernel.setArg(2, maxLength);
		}
		else {
			kernel = cl::Kernel(program, "NormalizeVectorField");
		}
		
		
		kernel.setArg(0, *inputAccess->get3DImage());
        kernel.setArg(1, *outputAccess->get());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
        );
    }
}

void NormalizeVectorField::setMaxLength(float maxLength) {
	this->maxLength = maxLength;
}

void NormalizeVectorField::setUseMaxLength(bool useMaxLength) {
	this->useMaxLength = useMaxLength;
}

}


