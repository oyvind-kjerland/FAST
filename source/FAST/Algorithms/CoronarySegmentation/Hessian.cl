__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
__constant sampler_t interpolationSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
__constant sampler_t hpSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

#define LPOS(pos) pos.x+pos.y*get_global_size(0)+pos.z*get_global_size(0)*get_global_size(1)

#ifdef VECTORS_16BIT
//#define UNORM16_TO_FLOAT(v) (float)v / 65535.0f
//#define TDF_TYPE ushort
//#define FLOAT_TO_UNORM16(v) convert_ushort_sat_rte(v * 65535.0f)
#define UNORM16_TO_FLOAT(v) v
#define FLOAT_TO_UNORM16(v) v
#define TDF_TYPE float

#define VECTOR_FIELD_TYPE short
#define FLOAT_TO_SNORM16_3(vector) convert_short3_sat_rte(vector * 32767.0f)

#else
#define UNORM16_TO_FLOAT(v) v
#define FLOAT_TO_UNORM16(v) v
#define TDF_TYPE float

#define VECTOR_FIELD_TYPE float
#define FLOAT_TO_SNORM16_3(vector) vector
#endif

float4 readImageToFloat(
        __read_only image3d_t volume, 
        int4 position
    ) {
    int dataType = get_image_channel_data_type(volume);
    float4 value;
    if(dataType == CLK_FLOAT) {
        value = read_imagef(volume, sampler, position).x; 
    } else if(dataType == CLK_SIGNED_INT16 || dataType == CLK_SIGNED_INT8) {
        value = convert_float4(read_imagei(volume, sampler, position)); 
    } else {
        value = convert_float4(read_imageui(volume, sampler, position)); 
    }
    return value;
}

float3 gradient(
        __read_only image3d_t volume,   // Volume to perform gradient on
        int4 pos,                       // Position to perform gradient on
        int volumeComponent,            // The volume component to perform gradient on: 0, 1 or 2
        int dimensions,                 // The number of dimensions to perform gradient in: 1, 2 or 3
        float3 spacing                  // Image spacing
    ) {
    float f100, f_100, f010 = 0, f0_10 = 0, f001 = 0, f00_1 = 0;
    switch(volumeComponent) {
        case 0:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).x; 
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).x;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).x; 
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).x;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).x;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).x;
        }
    break;
        case 1:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).y;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).y;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).y;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).y;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).y;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).y;
        }
    break;
        case 2:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).z;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).z;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).z;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).z;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).z;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).z;
        }
    break;
    }

    float3 gradient = {
        (f100-f_100)/(2.0f), 
        (f010-f0_10)/(2.0f),
        (f001-f00_1)/(2.0f)
    };
    
    // Keep original length
    float gradientLength = length(gradient);
    gradient /= spacing;
    gradient = gradientLength*normalize(gradient);
    

    return gradient;
}

float3 gradientNormalized(
        __read_only image3d_t volume,   // Volume to perform gradient on, this volume is vector field
        int4 pos,                       // Position to perform gradient on
        int volumeComponent,            // The volume component to perform gradient on: 0, 1 or 2
        int dimensions,                 // The number of dimensions to perform gradient in: 1, 2 or 3
        float3 spacing                  // Image spacing
    ) {
    float f100, f_100, f010 = 0, f0_10 = 0, f001 = 0, f00_1 = 0;
    switch(volumeComponent) {
        case 0:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).x; 
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).x;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).x; 
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).x;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).x;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).x;
        }
    break;
        case 1:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).y;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).y;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).y;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).y;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).y;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).y;
        }
    break;
        case 2:
        f100 = read_imagef(volume, pos + (int4)(1,0,0,0)).z;
        f_100 = read_imagef(volume, pos - (int4)(1,0,0,0)).z;
        if(dimensions > 1) {
            f010 = read_imagef(volume, pos + (int4)(0,1,0,0)).z;
            f0_10 = read_imagef(volume, pos - (int4)(0,1,0,0)).z;
        }
        if(dimensions > 2) {
            f001 = read_imagef(volume, pos + (int4)(0,0,1,0)).z;
            f00_1 = read_imagef(volume, pos - (int4)(0,0,1,0)).z;
        }
    break;
    }
    
    // Normalization
    f100 /= length(read_imagef(volume, sampler, pos + (int4)(1,0,0,0)).xyz);
    f_100 /= length(read_imagef(volume, sampler, pos - (int4)(1,0,0,0)).xyz);
    f010 /= length(read_imagef(volume, sampler, pos + (int4)(0,1,0,0)).xyz);
    f0_10 /= length(read_imagef(volume, sampler, pos - (int4)(0,1,0,0)).xyz);
    f001 /= length(read_imagef(volume, sampler, pos + (int4)(0,0,1,0)).xyz);
    f00_1 /= length(read_imagef(volume, sampler, pos - (int4)(0,0,1,0)).xyz);
    
    float3 gradient = {
        (f100 - f_100)/(2.0f),
        (f010 - f0_10)/(2.0f),
        (f001 - f00_1)/(2.0f)
    };
    
    
    // Keep original length
    /*
    float gradientLength = length(gradient);
    gradient /= spacing;
    gradient = gradientLength*normalize(gradient);
    */


    return gradient;
}

__kernel void toFloat(
        __read_only image3d_t volume,
#ifdef cl_khr_3d_image_writes_DISABLED
        __write_only image3d_t processedVolume,
#else
        __global float* processedVolume,
#endif
        __private float minimum,
        __private float maximum
        ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    
    float v = readImageToFloat(volume, pos).x;

    v = v > maximum ? maximum : v;
    v = v < minimum ? minimum : v;

    // Convert to floating point representation 0 to 1
    float value = (v - minimum) / (maximum - minimum);

    // Store value
#ifdef cl_khr_3d_image_writes_DISABLED
    write_imagef(processedVolume, pos, value);
#else
    processedVolume[LPOS(pos)] = value;
#endif
}

__kernel void createVectorField(
        __read_only image3d_t volume, 
#ifdef cl_khr_3d_image_writes_DISABLED
        __write_only image3d_t vectorField, 
#else
        __global VECTOR_FIELD_TYPE* vectorField,
#endif
        __private float Fmax,
        __private float vsign,
        __private float spacing_x,
        __private float spacing_y,
        __private float spacing_z
        ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    // Gradient of volume
    float4 F; 
    F.xyz = vsign*gradient(volume, pos, 0, 3, (float3)(spacing_x, spacing_y, spacing_z)); // The sign here is important
    F.w = 0.0f;

    // Fmax normalization
    const float l = length(F);
    F = l < Fmax ? F/(Fmax) : F / (l);
    F.w = 1.0f;

    // Store vector field
#ifdef cl_khr_3d_image_writes_DISABLED
    write_imagef(vectorField, pos, F);
#else
    vstore3(FLOAT_TO_SNORM16_3(F.xyz), LPOS(pos), vectorField);
#endif
}

// Forward declaration of eigen_decomp function
void eigen_decomposition(float M[3][3], float V[3][3], float e[3]);

__kernel void Hessian(
        __read_only image3d_t vectorField,
        __global float* outputEigenValues,
		__global float* outputTangents
    ) {
    const int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const float3 spacing = {1.0f, 1.0f, 1.0f};

    // Find Hessian Matrix
    float3 Fx, Fy, Fz;

	Fx = gradientNormalized(vectorField, pos, 0, 1, spacing);
	Fy = gradientNormalized(vectorField, pos, 1, 2, spacing);
	Fz = gradientNormalized(vectorField, pos, 2, 3, spacing);

    float Hessian[3][3] = {
        {Fx.x, Fy.x, Fz.x},
        {Fy.x, Fy.y, Fz.y},
        {Fz.x, Fz.y, Fz.z}
    };
    
    // Eigen decomposition
    float eigenValues[3];
    float eigenVectors[3][3];
    eigen_decomposition(Hessian, eigenVectors, eigenValues);
    
    float3 ht;
//	const float3 h = {eigenValues[0], eigenValues[1], eigenValues[2]};
	if (eigenValues[0] == eigenValues[0] && eigenValues[1] == eigenValues[1] && eigenValues[2] == eigenValues[2]) {
//		const float3 h = {eigenValues[0], eigenValues[1], eigenValues[2]};
		ht.x = eigenValues[0];
		ht.y = eigenValues[1];
		ht.z = eigenValues[2];
	} else {
		ht.x = 0;
		ht.y = 0;
		ht.z = 0;
	}

	const float3 h = (ht);
	const float3 t = {eigenVectors[0][0], eigenVectors[0][1], eigenVectors[0][2]};

	vstore3(h.xyz, LPOS(pos), outputEigenValues);
	vstore3(t.xyz, LPOS(pos), outputTangents);

}

int dsyevj3(float A[3][3], float Q[3][3], float w[3])
// ----------------------------------------------------------------------------
// Calculates the eigenvalues and normalized eigenvectors of a symmetric 3x3
// matrix A using the Jacobi algorithm.
// The upper triangular part of A is destroyed during the calculation,
// the diagonal elements are read but not destroyed, and the lower
// triangular elements are not referenced at all.
// ----------------------------------------------------------------------------
// Parameters:
//   A: The symmetric input matrix
//   Q: Storage buffer for eigenvectors
//   w: Storage buffer for eigenvalues
// ----------------------------------------------------------------------------
// Return value:
//   0: Success
//  -1: Error (no convergence)
// ----------------------------------------------------------------------------
{
  const int n = 3;
  float sd, so;                  // Sums of diagonal resp. off-diagonal elements
  float s, c, t;                 // sin(phi), cos(phi), tan(phi) and temporary storage
  float g, h, z, theta;          // More temporary storage
  float thresh;
  
  // Initialize Q to the identitity matrix
  for (int i=0; i < n; i++)
  {
    Q[i][i] = 1.0;
    for (int j=0; j < i; j++)
      Q[i][j] = Q[j][i] = 0.0;
  }

  // Initialize w to diag(A)
  for (int i=0; i < n; i++)
    w[i] = A[i][i];

  // Calculate SQR(tr(A))  
  sd = 0.0;
  for (int i=0; i < n; i++)
    sd += fabs(w[i]);
  sd = sd*sd;
 
  // Main iteration loop
  for (int nIter=0; nIter < 50; nIter++)
  {
    // Test for convergence 
    so = 0.0;
    for (int p=0; p < n; p++)
      for (int q=p+1; q < n; q++)
        so += fabs(A[p][q]);
    if (so == 0.0)
      return 0;

    if (nIter < 4) {
      thresh = 0.2 * so / (n*n);
    } else {
      thresh = 0.0;
    }

    // Do sweep
    for (int p=0; p < n; p++)
      for (int q=p+1; q < n; q++)
      {
        g = 100.0 * fabs(A[p][q]);
        if (nIter > 4  &&  fabs(w[p]) + g == fabs(w[p])
                       &&  fabs(w[q]) + g == fabs(w[q]))
        {
          A[p][q] = 0.0;
        }
        else if (fabs(A[p][q]) > thresh)
        {
          // Calculate Jacobi transformation
          h = w[q] - w[p];
          if (fabs(h) + g == fabs(h))
          {
            t = A[p][q] / h;
          }
          else
          {
            theta = 0.5 * h / A[p][q];
            if (theta < 0.0)
              t = -1.0 / (sqrt(1.0 + theta*theta) - theta);
            else
              t = 1.0 / (sqrt(1.0 + theta*theta) + theta);
          }
          c = 1.0/sqrt(1.0 + t*t);
          s = t * c;
          z = t * A[p][q];

          // Apply Jacobi transformation
          A[p][q] = 0.0;
          w[p] -= z;
          w[q] += z;
          for (int r=0; r < p; r++)
          {
            t = A[r][p];
            A[r][p] = c*t - s*A[r][q];
            A[r][q] = s*t + c*A[r][q];
          }
          for (int r=p+1; r < q; r++)
          {
            t = A[p][r];
            A[p][r] = c*t - s*A[r][q];
            A[r][q] = s*t + c*A[r][q];
          }
          for (int r=q+1; r < n; r++)
          {
            t = A[p][r];
            A[p][r] = c*t - s*A[q][r];
            A[q][r] = s*t + c*A[q][r];
          }

          // Update eigenvectors
          for (int r=0; r < n; r++)
          {
            t = Q[r][p];
            Q[r][p] = c*t - s*Q[r][q];
            Q[r][q] = s*t + c*Q[r][q];
          }
        }
      }
  }
  


  return -1;
}

void eigen_decomposition(float A[3][3], float V[3][3], float d[3]) {
  dsyevj3(A, V, d);

    // Sort eigenvalues and corresponding vectors.
  for (char i = 0; i < 3; ++i) {
    char k = i;
    float p = d[i];
    for (char j = i+1; j < 3; ++j) {
      if (fabs(d[j]) < fabs(p)) {
        k = j;
        p = d[j];
      }
    }
    if (k != i) {
      d[k] = d[i];
      d[i] = p;
      for (char j = 0; j < 3; ++j) {
        p = V[j][i];
        V[j][i] = V[j][k];
        V[j][k] = p;
      }
    }
  }
}
