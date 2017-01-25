__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

float readImageAsFloat2D(__read_only image2d_t image, sampler_t sampler, int2 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        return read_imagef(image, sampler, position).x;
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        return (float)read_imagei(image, sampler, position).x;
    } else {
        return (float)read_imageui(image, sampler, position).x;
    }
}

float readImageAsFloat3D(__read_only image3d_t image, sampler_t sampler, int4 position) {
    int dataType = get_image_channel_data_type(image);
    if(dataType == CLK_FLOAT || dataType == CLK_SNORM_INT16 || dataType == CLK_UNORM_INT16) {
        //return read_imagef(image, sampler, position).x;
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        return (float)read_imagei(image, sampler, position).x;
    } else if (dataType == CLK_UNSIGNED_INT16){
        return (float)read_imageui(image, sampler, position).x;
    }
}

__kernel void convertHU_2D(
	__read_only image2d_t input,
	__write_only image2d_t output,
	float minHU,
	float maxHU)
{	

	float gv, hu, hu_norm;
	const int2 pos = {get_global_id(0), get_global_id(1)};
	
	gv = readImageAsFloat2D(input, sampler, pos);
	hu = gv - 1024;
	
	if (hu > maxHU) {
		hu_norm = maxHU;
	} else if (hu < minHU){
		hu_norm = minHU;
	} else {
		hu_norm = (hu - minHU) / (maxHU - minHU);
	}
	write_imagef(output, pos, hu_norm);
}

__kernel void convertHU_3D(
	__read_only image3d_t input,
	__global float *output,
	__private float minimum,
	__private float maximum)
{	
	float gv, hu, hu_norm;
	const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
	
	gv = readImageAsFloat3D(input, sampler, pos);
	hu_norm = gv ;//- 1024;
	
	
	
	if (hu_norm > maximum) {
		hu_norm = maximum;
	} else if (hu_norm < minimum){
		hu_norm = minimum;
	}
	
	
	// Perform normalization?
	hu_norm = (hu_norm - minimum) / (maximum - minimum);
	
	
	output[LPOS(pos)] = gv;
	
}
	
