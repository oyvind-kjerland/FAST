#ifndef NORMALIZE_VECTOR_FIELD_HPP
#define NORMALIZE_VECTOR_FIELD_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class NormalizeVectorField : public ProcessObject {
    FAST_OBJECT(NormalizeVectorField);
    public:

		void setMaxLength(float maxLength);
		void setUseMaxLength(bool useMaxLength);

    private:
        NormalizeVectorField();
        void execute();

		bool useMaxLength;
		float maxLength;

};

}


#endif
