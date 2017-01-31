__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void NormalizeVectorField(
        __read_only image3d_t input,
        __global float* output
    ) {

	float3 vec, normalized;
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

	vec = read_imagef(input, sampler, pos).xyz;
	normalized = normalize(vec);

	vstore3(vec, LPOS(pos), output);
}

__kernel void NormalizeVectorFieldMax(
		__read_only image3d_t input,
		__global float* output,
		float maxLength
	) {
	
	float3 vec, normalized;
	float vecLength;
	
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	vec = read_imagef(input, sampler, pos).xyz;

	vecLength = length(vec);

	if (vecLength > maxLength) {
		normalized = vec / vecLength;
	} else {
		normalized = vec / maxLength;
	}
	
	vstore3(normalized, LPOS(pos), output);	
}