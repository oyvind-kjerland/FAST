#include "FAST/Algorithms/CoronarySegmentation/CreateTubeFromReference.hpp"
#include "FAST/DeviceManager.hpp"
#include <list>
#include <vector>
#include <fstream>
#include <iostream>

namespace fast {



CreateTubeFromReference::CreateTubeFromReference() {
	createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/CoronarySegmentation/CreateTubeFromReference.cl");

}

void CreateTubeFromReference::execute() {

    if(mFilename == "")
        throw Exception("No filename given to the RCAALineSetImporter");

	Image::pointer referenceImage = getStaticInputData<Image>(0);
    Image::pointer binaryVolume = getStaticOutputData<Image>(0);


    std::string buildOptions = "";
    DataType type = TYPE_FLOAT;
    int width = referenceImage->getWidth();
    int height = referenceImage->getHeight();
    int depth = referenceImage->getDepth();
    Vector3f spacing = referenceImage->getSpacing();

    mSpacing = spacing;

    binaryVolume->create(
			width,
			height,
			depth,
			type,
			1
	);
    binaryVolume->setSpacing(spacing);


    std::vector<ref_point> pointList;
    readReferencePoints(pointList);

    int num_points = pointList.size();
    std::cout << "pointList size: " << num_points << std::endl;

    Image::pointer points = Image::New();
    points->create(
    		num_points,
    		1,
    		1,
    		TYPE_FLOAT,
    		4
    );

    // Populate point image
    ImageAccess::pointer pointsAccess = points->getImageAccess(ACCESS_READ_WRITE);
    ref_point* pointsArray = (ref_point*)pointsAccess->get();

    for (int i=0; i<num_points; i++) {
    	pointsArray[i] = pointList[i];
    }
    
    // Release access
    pointsAccess->release();

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;


        OpenCLImageAccess::pointer pointsImageAccess = points->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer binaryVolumeAccess = binaryVolume->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
		kernel = cl::Kernel(program, "CreateTubeFromReference");
		
		int offset = 0;
		int chunk_size = 32;

		kernel.setArg(0, *pointsImageAccess->get3DImage());
		kernel.setArg(3, *binaryVolumeAccess->get());

		// Perform iterative
		while(1) {

			std::cout << "Offset: " << offset << std::endl;

			chunk_size = min(chunk_size, num_points - offset);

			kernel.setArg(1, offset);
			kernel.setArg(2, chunk_size);

	        device->getCommandQueue().enqueueNDRangeKernel(
	                kernel,
	                cl::NullRange,
	                cl::NDRange(width, height, depth),
	                cl::NullRange
	        );
	        device->getCommandQueue().finish();

	        offset += chunk_size;

	        if (offset >= num_points) {
	        	break;
	        }
		}

    }
}

void CreateTubeFromReference::readReferencePoints(std::vector<ref_point> &point_list) {

	for (int vessel=0; vessel<4; vessel++) {
		if(mFilename == "")
			throw Exception("No filename given to the RCAALineSetImporter");
		std::string tmpFilename = mFilename+"vessel" + std::to_string(vessel) + "/reference.txt";
		// Try to open file and check that it exists
		std::ifstream file(tmpFilename.c_str());
		if(!file.is_open()) {
			throw FileNotFoundException(tmpFilename);
		}

		// Add data to output
		std::string line;
		float x, y, z, r;
		int count = 0;

		std::ifstream referenceFile (tmpFilename);

		std::cout << tmpFilename << std::endl;
		if (referenceFile.is_open()) {
			while ( std::getline (referenceFile,line) )
			{
				std::istringstream iss(line);
				iss >> x >> y >> z >> r;


				x /= mSpacing[0];
				y /= mSpacing[1];
				z /= mSpacing[2];
				r /= mSpacing[0];


				ref_point rp;
				rp.x = x;
				rp.y = y;
				rp.z = z;
				rp.r = r;

				point_list.push_back(rp);
			}
			referenceFile.close();
		} else {
			std::cout << "Unable to open file" << std::endl;
		}
	}
	std::cout << "Read points" << std::endl;
}

void CreateTubeFromReference::setFilename(std::string filename) {
	mFilename = filename;
}

}


