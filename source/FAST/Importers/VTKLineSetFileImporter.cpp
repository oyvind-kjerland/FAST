#include "VTKLineSetFileImporter.hpp"
#include "FAST/Data/LineSet.hpp"
#include "FAST/Utility.hpp"
#include <fstream>

namespace fast {

void VTKLineSetFileImporter::setFilename(std::string filename) {
    mFilename = filename;
}

VTKLineSetFileImporter::VTKLineSetFileImporter() {
    mFilename = "";
    createOutputPort<LineSet>(0, OUTPUT_STATIC);
    mIsModified = true;
}

inline bool gotoLineWithString(std::ifstream &file, std::string searchFor) {
    bool found = false;
    std::string line;
    while(getline(file, line)) {
        if(line.find(searchFor) != std::string::npos) {
            found = true;
            break;
        }
    }

    return found;
}

void VTKLineSetFileImporter::execute() {
    if(mFilename == "")
        throw Exception("No filename given to the VTKLineSetFileImporter");

    // Try to open file and check that it exists
    std::ifstream file(mFilename.c_str());
    std::string line;
    if(!file.is_open()) {
        throw FileNotFoundException(mFilename);
    }

    // Check file header?

    // Read vertices
    std::vector<Vector3f> vertices;
    if(!gotoLineWithString(file, "POINTS")) {
        throw Exception("Found no points in the VTK file");
    }
    while(getline(file, line)) {
        trim(line);
        if(line.size() == 0)
            break;

        if(!(isdigit(line[0]) || line[0] == '-')) {
            // Has reached end
            break;
        }

        std::vector<std::string> tokens = split(line);

        for(int i = 0; i < tokens.size(); i += 3) {
            Vector3f v;
            v(0) = std::stof(tokens[i]);
            v(1) = std::stof(tokens[i+1]);
            v(2) = std::stof(tokens[i+2]);
            vertices.push_back(v);
        }
    }
    file.seekg(0); // set stream to start

    // Read lines
    std::vector<Vector2ui> lines;
    if(!gotoLineWithString(file, "LINES")) {
        throw Exception("Found no lines in the VTK file");
    }
    while(getline(file, line)) {
        trim(line);
        if(line.size() == 0)
            break;

        if(!(isdigit(line[0]) || line[0] == '-')) {
            // Has reached end
            break;
        }

        std::vector<std::string> tokens = split(line);

        if(tokens.size() != 3) {
            throw Exception("Error while reading lines in VTKLineSetFileImporter. Check format.");
        }

        Vector2ui line;
        line(0) = std::stoi(tokens[1]);
        line(1) = std::stoi(tokens[2]);

        lines.push_back(line);
    }

    // Add data to output
    LineSet::pointer output = getOutputData<LineSet>(0);
    output->create(vertices, lines);
}


} // end namespace fast

