__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void CreateTubeFromReference(
        __read_only image3d_t points,
        __private int offset,
        __private int length,
        __global float* binaryVolume
    ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

	float label = binaryVolume[LPOS(pos)];

	float4 point;
	float d2;
	float r2;
	int4 pointPos = {0,0,0,0};
	
	for (int i=offset; i<offset+length; i++) {
		pointPos.x = i;
		float4 point = read_imagef(points, sampler, pointPos).xyzw;
		// calculate distance
		d2 = pow(point.x - pos.x, 2) + pow(point.y - pos.y, 2) + pow(point.z - pos.z, 2);
		if (d2 < pow(point.w,2)) label = 1;
	}
	
	binaryVolume[LPOS(pos)] = label;

}