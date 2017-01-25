#ifndef MAX_TDF_FIELD_HPP
#define MAX_TDF_FIELD_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class MaxTDF : public ProcessObject {
    FAST_OBJECT(MaxTDF);
    public:

    private:
        MaxTDF();
        void execute();

};

}


#endif
