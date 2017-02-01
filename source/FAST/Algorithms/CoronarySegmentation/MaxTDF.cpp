#include "FAST/Algorithms/CoronarySegmentation/MaxTDF.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

MaxTDF::MaxTDF() {
    createInputPort<Image>(0);
    createInputPort<Image>(1);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/MaxTDF.cl");

}

void MaxTDF::execute() {
    Image::pointer input1 = getStaticInputData<Image>(0);
    Image::pointer input2 = getStaticInputData<Image>(1);
    Image::pointer output = getStaticOutputData<Image>(0);

    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;

    int width = input1->getWidth();
    int height = input1->getHeight();
    int depth = input1->getDepth();

	 output->create(
			width,
			height,
			depth,
			type,
			1
	);
    

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;

        OpenCLImageAccess::pointer inputAccess1 = input1->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer inputAccess2 = input2->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);


		kernel = cl::Kernel(program, "MaxTDF");
		
		kernel.setArg(0, *inputAccess1->get3DImage());
		kernel.setArg(1, *inputAccess2->get3DImage());
        kernel.setArg(2, *outputAccess->get());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width, height, depth),
                cl::NullRange
        );
    }
}

}


