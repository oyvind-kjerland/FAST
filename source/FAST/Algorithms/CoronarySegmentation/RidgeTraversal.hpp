#ifndef RIDGE_TRAVERSAL_HPP
#define RIDGE_TRAVERSAL_HPP

#include "FAST/ProcessObject.hpp"

#define RIDGE_TRAVERSAL_INPUT_TDF 0
#define RIDGE_TRAVERSAL_INPUT_TANGENTS 1
#define RIDGE_TRAVERSAL_INPUT_CANDIDATES 2
#define RIDGE_TRAVERSAL_OUTPUT_NEIGHBORS 0

namespace fast {

class RidgeTraversal : public ProcessObject {
    FAST_OBJECT(RidgeTraversal);
    public:

    	void setTLow(float t_low);
    	void setLMin(int l_min);
    	void setTDFInputConnection(ProcessObjectPort port);
    	void setTangentsInputConnection(ProcessObjectPort port);
    	void setRidgeCandidatesInputConnection(ProcessObjectPort port);

    	ProcessObjectPort getNeighborsOutputPort();
    private:
        RidgeTraversal();
        void execute();
        float t_low;
        int l_min;
};

}


#endif
