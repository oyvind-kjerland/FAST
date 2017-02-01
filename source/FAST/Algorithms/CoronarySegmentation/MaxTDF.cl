__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void MaxTDF(
        __read_only image3d_t tdf1,
		__read_only image3d_t tdf2,
		__read_only image3d_t tangents1,
		__read_only image3d_t tangents2,
        __global float* tdfOutput,
        __global float* tangentOutput
    ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

	float v;
	float3 t;

	float v1 = read_imagef(tdf1, sampler, pos).x;
	float v2 = read_imagef(tdf2, sampler, pos).x;
	
	float3 t1 = read_imagef(tangents1, sampler, pos).xyz;
	float3 t2 = read_imagef(tangents2, sampler, pos).xyz;


	if (v1 > v2) {
		v = v1;
		t = t1;
	} else {
		v = v2;
		t = t2;
	}
	
	tdfOutput[LPOS(pos)] = v;
	vstore3(t.xyz, LPOS(pos), tangentOutput);

}