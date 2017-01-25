/*
 * RCAALineSetImporter.hpp
 *
 *  Created on: Nov 11, 2016
 *      Author: oyvind
 */

#ifndef RCAALINESETIMPORTER_HPP_
#define RCAALINESETIMPORTER_HPP_


#include "Importer.hpp"

namespace fast {

class RCAALineSetImporter : public Importer {
    FAST_OBJECT(RCAALineSetImporter)
    public:
        void setFilename(std::string filename);
		void setSpacing(Vector3f spacing);
    private:
        RCAALineSetImporter();
        void execute();

		Vector3f mSpacing;
        std::string mFilename;
};

}


#endif /* RCAALINESETIMPORTER_HPP_ */
