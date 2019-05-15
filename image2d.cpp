#include "image2d.h"

using namespace std;

Image2D::Image2D() {
	//! Constructor
#ifdef LIBTIFF
	resolutionUnit = RESUNIT_NONE;
#endif
	xRes = 0.0;
	yRes = 0.0;
}

void Image2D::setResolution( const double setXRes, const double setYRes, const short setResolutionUnit ) {
	//! 
	resolutionUnit = setResolutionUnit;
	xRes = setXRes;
	yRes = setYRes;
}

int Image2D::writeTIFF( const string&  filename, //!< Name of the file to be written.
            const UINT32  width,                 //!< Image width (pixel).
            const UINT32  height,                //!< Image height (pixel).
                    double* raster,              //!< Colour-data width*height*(3|1) [0.0..maxVal].
                    const double  maxVal,        //!< maximum value for grayscale (or each of the RGB-channels)
                    const bool    isRGB          //!< RGB or Grayscale
	) {
	//! Convert some double data and write to a TIFF file.

	uint8_t* rasterRGB = new uint8_t[width*height*3];

	if( isRGB ) {
		for( UINT32 i=0; i<width*height*3; i++ ) {
			// some precaution about under- and overflows:
			if( raster[i] < 0.0 ) {
				cout << "[Image2D::writeTIFF] warning: underflow." << endl;
				rasterRGB[i] = 0x0;
			} else if( raster[i] > maxVal ) {
				cout << "[Image2D::writeTIFF] warning: overflow." << endl;
				rasterRGB[i] = 0xF;
			} else {
				rasterRGB[i] = floor( 255.0 * raster[i] / maxVal );
			}
		}
	} else {
		for( UINT32 i=0; i<width*height; i++ ) {
			// some precaution about under- and overflows:
			if( raster[i] < 0.0 ) {
				cout << "[Image2D::writeTIFF] warning: underflow: " << raster[i] << endl;
				rasterRGB[i] = 0x00;
			} else if( raster[i] > maxVal ) {
				cout << "[Image2D::writeTIFF] warning: overflow: " << raster[i] << endl;
				rasterRGB[i] = 0xFF;
			} else {
				rasterRGB[i] = floor( 255.0 * raster[i] / maxVal );
			}
		}
	}

	return writeTIFF( filename, width, height, rasterRGB, isRGB );

	delete[] rasterRGB;
}

//! Convert some float data and write to a TIFF file.
//! When both minVal and maxVal are a number (in contrast to not-a-number)
//! the values of raster will be normalized.
//!
//! Shows a warning in case of over- and underflows.
//! Sets pixels with under-/overflow to black/white
int Image2D::writeTIFF( const string& filename, //!< Name of the file to be written.
            const UINT32 width,                 //!< Image width (pixel).
            const UINT32 height,                //!< Image height (pixel).
                    float* raster,              //!< Colour-data width*height*(3|1) [minVal...maxVal].
                    const float  minVal,        //!< minimum Value (for normalization to [0...255]
                    const float  maxVal,        //!< minimum Value (for normalization to [0...255]
                    const bool   isRGB)          //!< RGB or Grayscale
{
	//cout << "[Image2D::writeTIFF] FLOAT" << endl;

	char*  rasterArray;
	UINT32 rasterArraySize;
	bool   normalize = false;
	float  range = 0.0;
	float  newValue;
	bool   warnOverFlow = false;
	bool   warnUnderFlow = false;

	// set normalization
	if( !isnan( minVal ) && !isnan( maxVal ) ) {
		range = maxVal - minVal;
		normalize = true;
		if( range <= 0.0 ) {
			cerr << __PRETTY_FUNCTION__ << " Normalization can not be performed: minVal > maxVal!";
			normalize = false;
		}
	}
	// set proper size
	if( isRGB ) {
		rasterArraySize = width*height*3;
	} else {
		rasterArraySize = width*height;
	}
	// allocate memory
	rasterArray = static_cast<char*>(calloc( rasterArraySize, sizeof(char) ));
	// fill/convert char array:
	for( UINT32 i=0; i<rasterArraySize; i++ ) {
		if( normalize ) {
			newValue = floor( CHAR_MAX * ( ( raster[i]-minVal ) / range ) + 0.5 );
		} else {
			newValue = floor( raster[i] + 0.5 );
		}
		if( newValue < 0.0 ) {
			newValue      = 0;
			warnUnderFlow = true;
		}
		if( newValue > 255.0 ) {
			newValue     = 255;
			warnOverFlow = true;
		}
		rasterArray[i] = newValue;
	}
	if( warnOverFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Overflow Warning, value > 255 found!";
	}
	if( warnUnderFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Underflow Warning, value < 0 found!";
	}

	bool retVal = writeTIFF( filename, width, height, rasterArray, isRGB );
	free( rasterArray );

	return retVal;
}

int Image2D::writeTIFF( const string&  filename, //!< Name of the file to be written.
			UINT32  width,    //!< Image width (pixel).
			UINT32  height,   //!< Image height (pixel).
	                double* raster,   //!< Colour-data width*height*(3|1) [minVal...maxVal].
	                double  minVal,   //!< minimum Value (for normalization to [0...255]
	                double  maxVal,   //!< minimum Value (for normalization to [0...255]
	                bool    isRGB     //!< RGB or Grayscale
	) {
	//! Convert some double data and write to a TIFF file.
	//! When both minVal and maxVal are a number (in contrast to not-a-number) 
	//! the values of raster will be normalized.
	//!
	//! Shows a warning in case of over- and underflows.
	//! Sets pixels with under-/overflow to black/white 

	//cout << "[Image2D::writeTIFF] DOUBLE" << endl;

	char*   rasterArray;
	UINT32  rasterArraySize;
	bool    normalize = false;
	double  range = 0.0;
	double  newValue;
	bool    warnOverFlow = false;
	bool    warnUnderFlow = false;

	// set normalization
	if( !std::isnan( minVal ) && !std::isnan( maxVal ) ) {
		range = maxVal - minVal;
		normalize = true;
		if( range <= 0.0 ) {
			cerr << __PRETTY_FUNCTION__ << " Normalization can not be performed: minVal > maxVal!";
			normalize = false;
		}
	}
	// set proper size
	if( isRGB ) {
		rasterArraySize = width*height*3;
	} else {
		rasterArraySize = width*height;
	}
	// allocate memory
	rasterArray = static_cast<char*>(calloc( rasterArraySize, sizeof(char) ));
	// fill/convert char array:
	for( UINT32 i=0; i<rasterArraySize; i++ ) {
		if( normalize ) {
			newValue = floor( CHAR_MAX * ( ( raster[i]-minVal ) / range ) + 0.5 );
		} else {
			newValue = floor( raster[i] + 0.5 );
		}
		if( newValue < 0.0 ) {
			newValue      = 0;
			warnUnderFlow = true;
		}
		if( newValue > 255.0 ) {
			newValue     = 255;
			warnOverFlow = true;
		}
		rasterArray[i] = newValue;
	}
	if( warnOverFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Overflow Warning, value > 255 found!";
	}
	if( warnUnderFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Underflow Warning, value < 0 found!";
	}

	bool retVal = writeTIFF( filename, width, height, rasterArray, isRGB );
	free( rasterArray );

	return retVal;
}

int Image2D::writeTIFF( string filename, //!< Name of the file to be written.
			UINT32 width,    //!< Image width (pixel).
			UINT32 height,   //!< Image height (pixel).
			char*  raster,   //!< Colour-data width*height*(3|1) [0..255].
			bool   isRGB     //!< RGB or Grayscale
	) {
	//! Write some data to a TIFF file.
	//!
	//! and more important a working real-world example: 
	//! for a binary-image: http://www.ibm.com/developerworks/linux/library/l-libtiff/
	//! for a colour-image: http://www.ibm.com/developerworks/linux/library/l-libtiff2/

#ifndef LIBTIFF
	cerr << "[Image2D::" << __FUNCTION__ << "] ERROR: libtiff NOT present!" << endl;
	return  _WRITE_ERROR_;
#else
	size_t foundDot;
	foundDot = filename.rfind( '.' );
	if( foundDot == string::npos ) {
		filename += ".tif";
	}

	TIFF* image = TIFFOpen( filename.c_str(), "w" );
	if( image == nullptr ) {
		cerr << "[Image2D] Could not open file: '" << filename << "'." << endl;		
		return _WRITE_ERROR_;
	} else {
		cout << "[Image2D] File open for writing: '" << filename << "'." << endl;
	}

	// We need to set some values for basic tags before we can add any data
	TIFFSetField( image, TIFFTAG_IMAGEWIDTH,      width );
	TIFFSetField( image, TIFFTAG_IMAGELENGTH,     height );
	// see http://www.libtiff.org/support.html
	TIFFSetField( image, TIFFTAG_COMPRESSION,     COMPRESSION_LZW ); // COMPRESSION_DEFLATE uses ZIP - see: http://www.awaresystems.be/imaging/tiff/tifftags/compression.html
	TIFFSetField( image, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG );
	TIFFSetField( image, TIFFTAG_BITSPERSAMPLE,   8 );
	if( resolutionUnit != RESUNIT_NONE ) {
		TIFFSetField( image, TIFFTAG_XRESOLUTION,    xRes );
		TIFFSetField( image, TIFFTAG_YRESOLUTION,    yRes );
		TIFFSetField( image, TIFFTAG_RESOLUTIONUNIT, resolutionUnit );
		//cout << "[Image2D] resolutionUnit " << resolutionUnit << " " << xRes << " " << yRes << endl;
	}
	if( isRGB ) {
		TIFFSetField( image, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB );
		TIFFSetField( image, TIFFTAG_SAMPLESPERPIXEL, 3 );
		// Actually write the RGB information to the file:
		if( TIFFWriteEncodedStrip( image, 0, raster, width * height * 3 ) == 0 ){
			cerr << "[Image2D] Could not write tiff image!" << endl;
		}
	} else {
		TIFFSetField( image, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_MINISBLACK );
		TIFFSetField( image, TIFFTAG_SAMPLESPERPIXEL, 1 );
		// Actually write the grayscale information to the file:
		if( TIFFWriteEncodedStrip( image, 0, raster, width * height ) == 0 ){
			cerr << "[Image2D] Could not write tiff image!" << endl;
		}
	}
	// Close the file
	TIFFClose( image );

	return  _WRITE_OK_;
#endif
}

int Image2D::writeTIFF( const string& filename,       //!< Name of the file to be written.
			UINT32 width,          //!< Image width (pixel).
			UINT32 height,         //!< Image height (pixel).
	                unsigned char* raster, //!< Colour-data width*height*(3|1) [0..255].
	                bool isRGB             //!< RGB or Grayscale
	) {
	//! Write some unsigned data to a TIFF file.
	return writeTIFF( filename, width, height, reinterpret_cast<char*>(raster), isRGB );
}


int Image2D::writeTIFFStack( const string& filename, UINT32 width, UINT32 height, UINT32 stackheight, char* imageStack, bool isRGB ) {
	//! Write a some 3D-data into a stack of single images.
	//!
	//! A sequence number and the file extension will be added.
	char strNr[12];
	for( UINT32 i=0; i<stackheight; i++ ) {
		sprintf( strNr, "%03i", i );
		writeTIFF( filename + "_stack_" + strNr, width, height, &imageStack[i*width*height], isRGB );
	}
	return 0;
}

int Image2D::writeTIFFStack( const string& filename, UINT32 width, UINT32 height, UINT32 stackheight, unsigned char* imageStack, bool isRGB ) {
	//! Write a some unsigned 3D-data into a stack of single images.
	//!
	//! A sequence number and the file extension will be added.
	char strNr[12];
	for( UINT32 i=0; i<stackheight; i++ ) {
		sprintf( strNr, "%03i", i );
		writeTIFF( filename + "_stack_" + strNr, width, height, &imageStack[i*width*height], isRGB );
	}
	return 0;
}

