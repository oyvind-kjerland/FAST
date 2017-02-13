__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void RidgeCandidateSelection(
        __read_only image3d_t tdf,
        __global float* candidates,
        __global float* neighbors,
        float t_high,
        float t_low,
        float t_max
    ) {

    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

	// Check if the current voxel is the maximum in the 3x3x3 neighborhood
	float current_value = read_imagef(tdf, sampler, pos).x;
	float is_candidate = 1;
	float neighbor_value;
	float max_neighborvalue = 0;
	
	if (current_value < t_max * t_high) is_candidate = 0;
	
	int4 offset = {0, 0, 0, 0};
	
	for (int z=0; z<3; z++) {
		for (int y=0; y<3; y++) {
			for (int x=0; x<3; x++) {
				offset = (int4)(pos.x+x-1, pos.y+y-1, pos.z+z-1, 0);
				// Check if a neighbor is maximum
				neighbor_value = read_imagef(tdf, sampler, offset).x;
				
				if (neighbor_value > current_value) {
					if (current_value > t_low * t_max) {
						is_candidate = 0.1;
					} else {
						is_candidate = 0;
					}
				}
			}
		}
	}
	
	candidates[LPOS(pos)] = is_candidate;

}