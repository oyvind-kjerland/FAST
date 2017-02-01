#ifndef MAX_TDF_FIELD_HPP
#define MAX_TDF_FIELD_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class MaxTDF : public ProcessObject {
    FAST_OBJECT(MaxTDF);
    public:
    	void setTDFInputConnection(int index, ProcessObjectPort port);
    	void setTangentsInputConnection(int index, ProcessObjectPort port);
    private:
        MaxTDF();
        void execute();

};

}


#endif
