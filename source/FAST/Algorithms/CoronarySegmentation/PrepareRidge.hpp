#ifndef PREPARE_RIDGE_HPP
#define PREPARE_RIDGE_HPP

#include "FAST/ProcessObject.hpp"

namespace fast {

class PrepareRidge : public ProcessObject {
    FAST_OBJECT(PrepareRidge);
    public:

    private:
        PrepareRidge();
        void execute();

};

}


#endif
