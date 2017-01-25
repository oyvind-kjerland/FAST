#ifndef HESSIAN_HPP
#define HESSIAN_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class Hessian : public ProcessObject {
    FAST_OBJECT(Hessian);
    public:

		ProcessObjectPort getEigenvaluesOuptutPort();
		ProcessObjectPort getTangentsOutputPort();

    private:
        Hessian();
        void execute();

        bool mUse16bitFormat;
};

}


#endif
