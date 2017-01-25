__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void MaxTDF(
        __read_only image3d_t inputImageGradient,
		__read_only image3d_t inputGradientVectorFlow,
        __global float* output
    ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

	float v1 = read_imagef(inputImageGradient, sampler, pos).x;
	float v2 = read_imagef(inputGradientVectorFlow, sampler, pos).x;

	if (v1 > v2) {
		output[LPOS(pos)] = v1;	
	} else {
		output[LPOS(pos)] = v2;
	}

	output[LPOS(pos)] = (v1 > v2) ? v1 : v2;

	vstore3(vec, LPOS(pos), output);
}