#include "FAST/Algorithms/CoronarySegmentation/MaxTDF.hpp"
#include "FAST/DeviceManager.hpp"

namespace fast {

MaxTDF::MaxTDF() {
    createInputPort<Image>(0);
    createInputPort<Image>(1);
    createInputPort<Image>(2);
    createInputPort<Image>(3);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOutputPort<Image>(1, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/MaxTDF.cl");

}

void MaxTDF::execute() {
    Image::pointer tdf1 = getStaticInputData<Image>(0);
    Image::pointer tdf2 = getStaticInputData<Image>(1);
    Image::pointer tangents1 = getStaticInputData<Image>(2);
    Image::pointer tangents2 = getStaticInputData<Image>(3);
    Image::pointer tdfOutput = getStaticOutputData<Image>(0);
    Image::pointer tangentsOutput = getStaticOutputData<Image>(1);

    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;
    int width = tdf1->getWidth();
    int height = tdf1->getHeight();
    int depth = tdf1->getDepth();

	tdfOutput->create(
			width,
			height,
			depth,
			type,
			1
	);

	tangentsOutput->create(
			width,
			height,
			depth,
			type,
			3
	);
    

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;

        OpenCLImageAccess::pointer tdfAccess1 = tdf1->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer tdfAccess2 = tdf2->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer tangentsAccess1 = tangents1->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer tangentsAccess2 = tangents2->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer tdfOutputAccess = tdfOutput->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        OpenCLBufferAccess::pointer tangentsOutputAccess = tangentsOutput->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

		kernel = cl::Kernel(program, "MaxTDF");
		
		kernel.setArg(0, *tdfAccess1->get3DImage());
		kernel.setArg(1, *tdfAccess2->get3DImage());
		kernel.setArg(2, *tangentsAccess1->get3DImage());
		kernel.setArg(3, *tangentsAccess2->get3DImage());
        kernel.setArg(4, *tdfOutputAccess->get());
        kernel.setArg(5, *tangentsOutputAccess->get());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(width, height, depth),
                cl::NullRange
        );
    }
}

void MaxTDF::setTDFInputConnection(int index, ProcessObjectPort port) {
	if (index == 0 || index == 1) {
		setInputConnection(index, port);
	} else {
		throw Exception("TDF input only have two ports, 0 and 1");
	}
}
void MaxTDF::setTangentsInputConnection(int index, ProcessObjectPort port) {
	if (index == 0 || index == 1) {
		setInputConnection(index+2, port);
	} else {
		throw Exception("Tangents input only have two ports, 0 and 1");
	}
}

}


