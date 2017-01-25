/*
 * MedianFilter.hpp
 *
 *  Created on: Nov 23, 2016
 *      Author: oyvind
 */

#ifndef MEDIANFILTER_HPP_
#define MEDIANFILTER_HPP_

#include "FAST/ProcessObject.hpp"


using namespace fast;

class MedianFilter : public ProcessObject {
	FAST_OBJECT(MedianFilter);
	public:
		void setKernelSize(int size);
		~MedianFilter();
	private:
		MedianFilter();

		void execute();

		int kernelSize;

};



#endif /* MEDIANFILTER_HPP_ */
