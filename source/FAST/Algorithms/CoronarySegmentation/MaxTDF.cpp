#include "FAST/Algorithms/CoronarySegmentation/MaxTDF.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

MaxTDF::MaxTDF() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/MaxTDF.cl");

}

void MaxTDF::execute() {
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


		kernel = cl::Kernel(program, "MaxTDF");
		
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

}


