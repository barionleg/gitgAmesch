#ifndef IMAGE2D_H
#define IMAGE2D_H

// C++ includes:
#include "gmcommon.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib> // uint, callo

#ifdef LIBTIFF
	#include "tiffio.h"
	#define UINT32 uint32
	// for insights about uint32 visit: http://stackoverflow.com/questions/911035/uint32-int16-and-the-like-are-they-standard-c
#else
	#define UINT32 unsigned int
	#define RESUNIT_CENTIMETER 0
#endif

// identical to meshio:

#define _READ_OK_      (0)
#define _READ_ERROR_   (-1)

#define _WRITE_OK_     (0)
#define _WRITE_ERROR_  (-1)

//!
//! \brief Class handling 2D images. (Layer -1)
//!
//! This class should be used to access 2D images. (Layer -1)
//!
//! Requires: libtiff
//! see: http://www.libtiff.org
//!
//! Layer -1
//!

class Image2D {
	//! \todo: collect common flags from image2d.h and meshio.h (_READ_..., etc.)
	
	public:
		// constructor and deconstructor:
		Image2D();
		~Image2D() = default;

		void setResolution(const double xRes, const double yRes, const short resolutionUnit=RESUNIT_CENTIMETER );

		int writeTIFF(const std::string& filename, UINT32 width, UINT32 height, double* raster, double maxVal, bool isRGB=true );
		int writeTIFF(const std::string& filename, UINT32 width, UINT32 height, float* raster,  float minVal=_NOT_A_NUMBER_,  float maxVal=_NOT_A_NUMBER_,  bool isRGB=true );
		int writeTIFF(const std::string& filename, UINT32 width, UINT32 height, double* raster, double minVal=_NOT_A_NUMBER_, double maxVal=_NOT_A_NUMBER_, bool isRGB=true );
		int writeTIFF(std::string filename, UINT32 width, UINT32 height, char* raster,          bool isRGB=true );
		int writeTIFF(const std::string& filename, UINT32 width, UINT32 height, unsigned char* raster, bool isRGB=true );
		int writeTIFFStack(const std::string& filename, UINT32 width, UINT32 height, UINT32 stackheight, char* imageStack, bool isRGB=true );
		int writeTIFFStack(const std::string& filename, UINT32 width, UINT32 height, UINT32 stackheight, unsigned char* imageStack, bool isRGB=true );

	private:
		short  resolutionUnit; //!< None, Inch (DPI), Centimeter
		double xRes;           //!< dots per resolutionUnit in x
		double yRes;           //!< dots per resolutionUnit in y
};

#endif
