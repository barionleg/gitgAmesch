// example1.cpp 
  
#include <stdio.h>  
#include <cuda.h>  
  

__global__ void cudaKernelSquareArray( float *a, int N ) {  
	//! Kernel that executes on the CUDA device to square an array.
	//! Used for simple testing.
	int idx = blockIdx.x * blockDim.x + threadIdx.x;  
	if (idx<N) a[idx] = a[idx] * a[idx];  
}  

void cudaSimpleTest() {
  float *a_h, *a_d;  // Pointer to host & device arrays  
  const int N = 20;  // Number of elements in arrays  
  size_t size = N * sizeof(float);  
  a_h = (float *)malloc(size);        // Allocate array on host  
  cudaMalloc((void **) &a_d, size);   // Allocate array on device  

  // Initialize host array and copy it to CUDA device  
  for (int i=0; i<N; i++) a_h[i] = (float)i;  
  cudaMemcpy(a_d, a_h, size, cudaMemcpyHostToDevice);  

  // Do calculation on device:  
  int block_size = 4;  
  int n_blocks = N/block_size + (N%block_size == 0 ? 0:1);  
  cudaKernelSquareArray <<< n_blocks, block_size >>> (a_d, N);  

  // Retrieve result from device and store it in host array  
  cudaMemcpy(a_h, a_d, sizeof(float)*N, cudaMemcpyDeviceToHost);  

  // Print results  
  for (int i=0; i<N; i++) printf("%d %f\n", i, a_h[i]);  

  // Cleanup  
  free(a_h); cudaFree(a_d);  
}

__global__ void cudaKernelEstEdgeProps( float* coordArr, float* distArr, float* cogArr, int N ) {  
	//! Used for edges: (kernel executed on the CUDA device)
	//! 1. estimates the length of an edge (distance between two vertices A and B)
	//! 2. estimtes the center of gravity (vertex half-way between A and B)

	int idx = blockIdx.x * blockDim.x + threadIdx.x;  
	float dX, dY, dZ;

	if( idx < N ) {
		dX = coordArr[idx*6+3] - coordArr[idx*6];
		dY = coordArr[idx*6+4] - coordArr[idx*6+1];
		dZ = coordArr[idx*6+5] - coordArr[idx*6+2];
		distArr[idx] = sqrt( dX*dX + dY*dY + dZ*dZ );  
		cogArr[idx*3]   = ( coordArr[idx*6+3] + coordArr[idx*6] ) / 2;
		cogArr[idx*3+1] = ( coordArr[idx*6+4] + coordArr[idx*6+1] ) / 2;
		cogArr[idx*3+2] = ( coordArr[idx*6+5] + coordArr[idx*6+2] ) / 2;
	}
}  
  
void cudaEstimateEdgeProperties( float* coordArr, float* distArr, float* cogArr, int elementsNr ) {
	//! Used for edges: (public)
	//! 1. estimates the length of an edge (distance between two vertices A and B)
	//! 2. estimtes the center of gravity (vertex half-way between A and B)

	float* deviceCoordArr;
	float* deviceDistArr;
	float* deviceCogArr;
	const int N = elementsNr;
	size_t size = N * sizeof( float );  

	// Allocate array on device  
	cudaMalloc( (void **) &deviceCoordArr, size*6 );  
	cudaMalloc( (void **) &deviceDistArr, size );  
	cudaMalloc( (void **) &deviceCogArr, size );  

	// Copy coordinates to CUDA device
	cudaMemcpy( deviceCoordArr, coordArr, size*6, cudaMemcpyHostToDevice );  
 
	// Do calculation on device:  
	int block_size = 32;  
	int n_blocks = N/block_size + (N%block_size == 0 ? 0:1);  
	//printf( "block_size: %d\n", block_size );  
	//printf( "n_blocks:   %d\n", n_blocks );  
	cudaKernelEstEdgeProps <<< n_blocks, block_size >>> ( deviceCoordArr, deviceDistArr, deviceCogArr, N );  

	// Retrieve result from device and store it in host array  
	cudaMemcpy( distArr, deviceDistArr, sizeof( float ) * N, cudaMemcpyDeviceToHost );  
	cudaFree( deviceCoordArr );
	cudaFree( deviceDistArr );
}
