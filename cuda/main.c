#include <stdio.h>  
#include <stdlib.h> // calloc

#include "cudamesh.h"

int main(void) {  
	//! Main routine that executes on the host for testing our own CUDA library.
	//==========================================================================

	// just square an array and put the results to stdout:
	//--------------------------------------------------------------------------
	cudaSimpleTest();

	// estimate the lengths and centers of gravity of edges:
	//--------------------------------------------------------------------------
	int distNr = 8;
	//float* coords = (float*) calloc( distNr*6, sizeof( float ) );
	float coords[50] = {
	                     -13.0482, -3.4833, 4.2940, -13.0376, -3.4708, 4.2833,
                         1, 1, 0, 1, 1, 1, 
                         5, 5, 5, 7, 7, 7, 
                         5, 5, 5, 7, 7, 7, 
                         5, 5, 5, 7, 7, 7, 
                         5, 5, 5, 7, 7, 7, 
                         3, 3, 3, 8, 8, 8,
	                     -13.0482, -3.4833, 4.2940, -13.0376, -3.4708, 4.2833
	                   };
	float* dists  = (float*) calloc( distNr, sizeof( float ) ); 
	float* cogs   = (float*) calloc( distNr*3, sizeof( float ) ); 
	for( int i=0; i<distNr; i++ ) {
		dists[i] = -(i+1);
	}	

	cudaEstimateEdgeProperties( coords, dists, cogs, distNr );

	// Print results to stdout:
	for( int i=0; i<distNr; i++ ) {
		printf( "cudaEstimateEdgeProperties: %d %f %f\n", i, dists[i], coords[i*6] );  
	}
} 

