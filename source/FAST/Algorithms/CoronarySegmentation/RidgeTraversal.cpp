#include "FAST/Algorithms/CoronarySegmentation/RidgeTraversal.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/LineSet.hpp"
#include <queue>
#include <vector>
#include <list>
#include <stack>

#define ARRAY_POS(x,y,z) z*height*width+y*width+x
#define POINT_POS(n)
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

    // Forward index, backward index, same direction forward, same direction backward
    neighbors->create(
			width,
			height,
			depth,
			type,
			4
	);


    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel;


        // Calculate t_low
        float t_max = tdf->calculateMaximumIntensity();
        float t_low = this->t_low * t_max;

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

    std::cout << "Finish kernel" << std::endl;

    // Access candidates
    ImageAccess::pointer candidateAccess = candidates->getImageAccess(ACCESS_READ);
    float* candidateArray = (float*)candidateAccess->get();

    // Access neighbors
    ImageAccess::pointer neighborAccess = neighbors->getImageAccess(ACCESS_READ);
    ridge_neighbor* neighborArray = (ridge_neighbor*)neighborAccess->get();

    // Create centerline image
    Image::pointer centerLineImage = Image::New();
    centerLineImage->create(
			width,
			height,
			depth,
			type,
			1
	);
    // Init centerlineImage
    //centerLineImage->fill(0);

    ImageAccess::pointer centerLineImageAccess = centerLineImage->getImageAccess(ACCESS_READ_WRITE);
    float* centerLineImageArray = (float*)centerLineImageAccess->get();

    std::cout << "Initialize Centerline Image" << std::endl;

    for (int i=0; i<width*height*depth; i++) {
    	centerLineImageArray[i] = 0;
    }

    std::cout << "Centerline Image Initialized" << std::endl;

    // Traverse candidates
    int sum_candidates = 0;
    int length;
    ridge_neighbor neighbor;

    std::stack<int> validCandidateStack;

    int same_direction;
    for (int z=0; z<depth; z++) {
    	for (int y=0; y<height; y++) {
    		for (int x=0; x<width; x++) {
    			if (candidateArray[ARRAY_POS(x,y,z)] == 1) {

    				sum_candidates++;

    				length = 0;

    				// Traverse forward
    				neighbor = neighborArray[ARRAY_POS(x,y,z)];
    				same_direction = 1;

    				while (1) {

    					if (same_direction) {
    						if (neighbor.forward_pos == 0 || centerLineImageArray[(int)neighbor.forward_pos] == 1) break;
							length++;
    						centerLineImageArray[(int)neighbor.forward_pos] = 1;
    						same_direction = neighbor.forward_direction;
    						neighbor = neighborArray[(int)neighbor.forward_pos];

    					} else {
    						if (neighbor.backward_pos == 0 || centerLineImageArray[(int)neighbor.backward_pos] == 1) break;
    						length++;
    						centerLineImageArray[(int)neighbor.backward_pos] = 1;
    						same_direction = neighbor.backward_direction;
    						neighbor = neighborArray[(int)neighbor.backward_pos];
    					}
    				}

    				// Traverse backward
    				neighbor = neighborArray[ARRAY_POS(x,y,z)];
    				same_direction = 1;

    				while (1) {

    					if (same_direction) {
    						if (neighbor.backward_pos == 0 || centerLineImageArray[(int)neighbor.backward_pos] == 1) break;
							length++;
    						centerLineImageArray[(int)neighbor.backward_pos] = 1;
    						same_direction = neighbor.backward_direction;
    						neighbor = neighborArray[(int)neighbor.backward_pos];

    					} else {
    						if (neighbor.forward_pos == 0 || centerLineImageArray[(int)neighbor.forward_pos] == 1) break;
    						length++;
    						centerLineImageArray[(int)neighbor.forward_pos] = 1;
    						same_direction = neighbor.forward_direction;
    						neighbor = neighborArray[(int)neighbor.forward_pos];
    					}
    				}

    				if (length >= l_min) {
    					std::cout << "Ridge length" << length << std::endl;
    					validCandidateStack.push(ARRAY_POS(x,y,z));
    				}

    			}

    		}

    	}

    }

    /*
    // Extract lines
  	int candidate;
  	ridge_point rp;

    for (int i=0; i<validCandidateStack.size(); i++) {

    	candidate = validCandidateStack.top();
    	validCandidateStack.pop();

    	std::list<ridge_point> ridge_points;
    	rp = createPointFromArrayPos(candidate, width, height);
    	std::cout << "Ridge point, x: " << rp.x << " y: " << rp.y << " z: " << rp.z << std::endl;


		// Traverse forward
		neighbor = neighborArray[ARRAY_POS(x,y,z)];
		same_direction = 1;

		while (1) {

			if (same_direction) {
				if (neighbor.forward_pos == 0 || centerLineImageArray[(int)neighbor.forward_pos] == 1) break;
				length++;
				centerLineImageArray[(int)neighbor.forward_pos] = 1;
				same_direction = neighbor.forward_direction;
				neighbor = neighborArray[(int)neighbor.forward_pos];

			} else {
				if (neighbor.backward_pos == 0 || centerLineImageArray[(int)neighbor.backward_pos] == 1) break;
				length++;
				centerLineImageArray[(int)neighbor.backward_pos] = 1;
				same_direction = neighbor.backward_direction;
				neighbor = neighborArray[(int)neighbor.backward_pos];
			}
		}


    }*/




    std::cout << "Number of candidates: " << sum_candidates << " Num valid: " << validCandidateStack.size() << std::endl;





    std::cout << "Finish Execute RidgeTraversal" << std::endl;
}


ridge_point RidgeTraversal::createPointFromArrayPos(int arrayPos, int width, int height) {
	ridge_point rp;

	rp.x = arrayPos % width;
	rp.y = (arrayPos % (width*height)) / width;
	rp.z = arrayPos / (width*height);

	return rp;
}

void RidgeTraversal::setTLow(float t_low) {
	this->t_low = t_low;
}

void RidgeTraversal::setLMin(int l_min) {
	this->l_min = l_min;
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


