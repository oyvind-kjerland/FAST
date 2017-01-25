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
	
	float cube[125];
	

	
	
	for (int z=0; z<5; z++) {
		for (int y=0; y<5; y++) {
			for (int x=0; x<5; x++) {
				int4 offset = {x - 2, y - 2, z - 2, 0};
				cube[z*25+y*5+x] = read_imagef(input, sampler, pos+offset).x;
			}
		}
	}
	//output[LPOS(pos)] = 1;
	int over, under, side;
	float v, o;
	//o = 1;
	for (int i=0; i<125; i++) {
		over = 0;
		under = 0;
		side = 0;
		v = cube[i];
		for (int j=0; j<125; j++) {
			if (v > cube[j]) {
				under++;
			} else if (v < cube[j]) {
				over++;
			} else {
				side++;
			}
		}
		
		if (abs(over-under) <= side) {
			// Found median
			output[LPOS(pos)] = v;
			//o = 2;
		}
	}
	
	//output[LPOS(pos)] = o;
}
	