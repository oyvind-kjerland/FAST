__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void FrangiTDF3D(
        __read_only image3d_t inputEigenvalues,
        __global float* output,
		float a,
		float b,
		float c
    ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	float3 h = read_imagef(inputEigenvalues, sampler, pos).xyz;
	float h1 = h.x;
	float h2 = h.y;
	float h3 = h.z;
	
	float Ra = fabs(h1) / sqrt(fabs(h2)*fabs(h3));
	float Rb = fabs(h2) / fabs(h3);
	float S = sqrt(h1*h1 + h2*h2 + h3*h3);
	
	float t;
	
	if (h2 > 0 || h3 > 0) {
		t = 0;
	} else {
		a = 0.5;
		b = 0.5;
		c = 100;
		t = (1 - exp(-Ra*Ra/(2*a*a))) * exp(-Rb*Rb/(2*b*b)) * (1 - exp(-S*S/(2*c*c)));
	}
	
	if (t == t) {

	} else {
		t = 0;
	}

	output[LPOS(pos)] = t;
}
