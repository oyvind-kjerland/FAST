#ifndef FRANGI_TDF_HPP
#define FRANGI_TDF_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class FrangiTDF : public ProcessObject {
    FAST_OBJECT(FrangiTDF);
    public:

		void setTubeConstants(float alpha, float beta, float gamma);

    private:
        FrangiTDF();
        void execute();

		// Tube likeness constants
		float alpha;
		float beta;
		float gamma;

};

}


#endif
