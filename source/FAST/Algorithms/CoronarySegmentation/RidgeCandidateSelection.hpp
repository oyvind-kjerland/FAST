#ifndef RIDGE_CANDIDATE_SELECTION_HPP
#define RIDGE_CANDIDATE_SELECTION_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class RidgeCandidateSelection : public ProcessObject {
    FAST_OBJECT(RidgeCandidateSelection);
    public:

		ProcessObjectPort getCandidatesOuptutPort();
		ProcessObjectPort getNeighborsOutputPort();

		void setTHigh(float t_high);
		void setTLow(float t_low);

    private:
        RidgeCandidateSelection();
        void execute();

        float t_high;
        float t_low;
};

}


#endif
