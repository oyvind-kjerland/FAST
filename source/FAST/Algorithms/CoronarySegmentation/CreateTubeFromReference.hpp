#ifndef CREATE_TUBE_FROM_REFERENCE_HPP
#define CREATE_TUBE_FROM_REFERENCE_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"
#include <list>
#include <vector>

namespace fast {

typedef struct _ref_point {
	float x, y, z, r;
} ref_point;


class CreateTubeFromReference : public ProcessObject {
    FAST_OBJECT(CreateTubeFromReference);
    public:
    void setFilename(std::string filename);

    private:
        CreateTubeFromReference();
        void execute();

        std::string mFilename;
        Vector3f mSpacing;
        void readReferencePoints(std::vector<ref_point> &point_list);
};

}


#endif
