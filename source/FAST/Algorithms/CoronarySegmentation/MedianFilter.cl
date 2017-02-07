#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

// Should consider using sorting, as this will run through 125+125*125 iterations

__kernel void medianFilter_5x5x5(
	__read_only image3d_t input,
	__global float *output,
	int d)
{	
	//const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	const int4 pos = {get_global_id(0), get_global_id(1), d, 0};
	
	__private float cube[125];
	__private float outCube[125];

	
	
	for (int z=0; z<5; z++) {
		for (int y=0; y<5; y++) {
			for (int x=0; x<5; x++) {
				int4 offset = {x - 2, y - 2, z - 2, 0};
				cube[z*25+y*5+x] = read_imagef(input, sampler, pos+offset).x;
			}
		}
	}
	
	
	// sorting, in-place, n(n-1)/2
	float v, v_max;
	int v_j;
	for (int i=0; i<125; i++) {
		v_max = 0;
		v_j = 0;
		for (int j=0; j<125; j++) {
			if (cube[j] > v_max) {
				v_max = cube[j];
				v_j = j;
			}
		}
		outCube[i] = v_max;
		cube[v_j] = 0.0f;
	}
	
	// 125/2 + 1 = 63
	output[LPOS(pos)] = outCube[63];
	
	
	// median selection
	
//	int over, under, side;
//	float v, o;
//	//o = 1;
//	for (int i=0; i<125; i++) {
//		over = 0;
//		under = 0;
//		side = 0;
//		v = cube[i];
//		for (int j=0; j<125; j++) {
//			if (v > cube[j]) {
//				under++;
//			} else if (v < cube[j]) {
//				over++;
//			} else {
//				side++;
//			}
//		}
//		
//		if (abs(over-under) <= side) {
//			// Found median
//			output[LPOS(pos)] = v;
//			//o = 2;
//		}
//	}
	

}
	