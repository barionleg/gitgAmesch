#include <stdio.h>  
#include <stdlib.h> // calloc
#include <string>

#include "image2d.h"

using namespace std;

int main(void) {  
	//! Main routine for testing access to 2D-Images
	//==========================================================================
	Image2D someImage;
	// write a 5x4 TIFF with a red, green and blue pixle:
	char raster[ 5 * 4 * 3 ] = {
		0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0xF, 0x0, 0x0,  0x0, 0x0, 0x0,
		0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0xF, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,
		0x0, 0x0, 0x0,  0x0, 0x0, 0xF,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,
		0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0
	};
	someImage.writeTIFF( "test1.tiff", 5, 4, raster );

	// write a 100x50 TIFF with random colours:
	cout << "RAND_MAX: " << RAND_MAX << endl;
	srand( rand() );
	float rasterRGB[ 100*50*3 ];
	for( unsigned int i=0; i<(100*50*3); i++ ) { 
		rasterRGB[i] = (float)abs(rand())/RAND_MAX;
	}
	someImage.writeTIFF( "test2.tiff", 100, 50, rasterRGB );

	// write a 100x50 TIFF with random grayscale:
	cout << "RAND_MAX: " << RAND_MAX << endl;
	srand( rand() );
	float rasterGray[ 100*50 ];
	for( unsigned int i=0; i<(100*50); i++ ) { 
		rasterGray[i] = (float)abs(rand())/RAND_MAX;
	}
	someImage.writeTIFF( "test3.tiff", 100, 50, rasterGray, false );
}
