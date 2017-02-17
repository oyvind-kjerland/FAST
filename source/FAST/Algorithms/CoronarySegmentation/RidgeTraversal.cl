__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)


__kernel void FindNeighbors(
        __read_only image3d_t tdf,
        __read_only image3d_t tangents,
        __global float* neighbors,
		__private float Tlow
    ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	float max_forward_val = 0;
	int4 max_forward_pos = {-1,0,0,0};
	float max_backward_val = 0;
	int4 max_backward_pos = {-1,0,0,0}; 
	
	int4 tdf_pos = {0,0,0,0}; 
	float tdf_val;
	
	// Tangent
	float3 t = read_imagef(tangents, sampler, pos).xyz;
	float dot_product;
	
	// TODO consider hardcoding the 26 directions
	for (int z=-1; z<2; z++) {
		for (int y=-1; y<2; y++) {
			for (int x=-1; x<2; x++) {
				
				// Set neighbor TDF position
				tdf_pos.x = pos.x + x;
				tdf_pos.y = pos.y + y;
				tdf_pos.z = pos.z + z;
				
				// Read TDF value
				tdf_val = read_imagef(tdf, sampler, tdf_pos).x;
				
				// Calculate dot product
				dot_product = x*t.x + y*t.y + z*t.z;
				
				// Check forward
				if (dot_product > 0) {
					if (tdf_val > Tlow && tdf_val > max_forward_val) {
						max_forward_val = tdf_val;
						max_forward_pos.x = tdf_pos.x;
						max_forward_pos.y = tdf_pos.y;
						max_forward_pos.z = tdf_pos.z;  
					}
				// Check backward			
				} else if (dot_product < 0) {
					if (tdf_val > Tlow && tdf_val > max_backward_val) {
						max_backward_val = tdf_val;
						max_backward_pos.x = tdf_pos.x;
						max_backward_pos.y = tdf_pos.y;
						max_backward_pos.z = tdf_pos.z;  
					}
				}
			}
		}
	}
	
	float2 o = {LPOS(max_forward_pos), LPOS(max_backward_pos)};
	vstore2(o.xy, LPOS(pos), neighbors);
}
