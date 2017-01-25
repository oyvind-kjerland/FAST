#include "RCAALineSetImporter.hpp"
#include "FAST/Data/LineSet.hpp"
#include <fstream>
#include <iostream>

namespace fast {

void RCAALineSetImporter::setFilename(std::string filename) {
    mFilename = filename;
}

RCAALineSetImporter::RCAALineSetImporter() {
    mFilename = "";
    createOutputPort<LineSet>(0, OUTPUT_STATIC);
    mIsModified = true;
	mSpacing = {1,1,1};
}


void RCAALineSetImporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to the RCAALineSetImporter");

    // Try to open file and check that it exists
    std::ifstream file(mFilename.c_str());
    if(!file.is_open()) {
        throw FileNotFoundException(mFilename);
    }

    // Add data to output
    std::string line;
	float x, y, z;
	int count = 0;

	LineSet::pointer output = getOutputData<LineSet>(0);

	LineSetAccess::pointer linesetAccess = output->getAccess(ACCESS_READ_WRITE);

	std::ifstream referenceFile (mFilename);

	std::cout << mFilename << std::endl;
	if (referenceFile.is_open()) {
		while ( std::getline (referenceFile,line) )
		{
			std::istringstream iss(line);
			// Read float values and add to lineset
			iss >> x >> y >> z;
			Vector3f v(x / mSpacing[0], y / mSpacing[1], z / mSpacing[2]);
			
			linesetAccess->addPoint(v);
			count++;
		}
		referenceFile.close();
	} else {
		std::cout << "Unable to open file" << std::endl;
	}

	for (int i=0; i<count-1; i++) {
		linesetAccess->addLine(i,i+1);
	}
}

void RCAALineSetImporter::setSpacing(Vector3f spacing) {
	mSpacing = spacing;
}


} // end namespace fast


