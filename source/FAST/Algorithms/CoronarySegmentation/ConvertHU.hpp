/*
 * ConvertHU.hpp
 *
 *  Created on: Oct 29, 2016
 *      Author: oyvind
 */

#ifndef CONVERTHU_HPP_
#define CONVERTHU_HPP_

#include "FAST/ProcessObject.hpp"


using namespace fast;

class ConvertHU : public ProcessObject {
	FAST_OBJECT(ConvertHU);
	public:
		void setMaxHU(float maxHU);
		void setMinHU(float minHU);
		~ConvertHU();
	private:
		ConvertHU();

		void execute();

		float maxHU;
		float minHU;
};


#endif /* CONVERTHU_HPP_ */
