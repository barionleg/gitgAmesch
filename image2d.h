#ifndef IMAGE2D_H
#define IMAGE2D_H

#ifdef LIBTIFF
	#include "tiffio.h"
	// for insights about uint32_t visit: http://stackoverflow.com/questions/911035/uint32_t-int16-and-the-like-are-they-standard-c
#endif

// C++ includes:
#include "gmcommon.h"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib> // uint, callo

// identical to meshio:

//!
//! \brief Class handling 2D images. (Layer -1)
//!
//! This class should be used to access 2D images. (Layer -1)
//!
//! Requires: libtiff
//! see: http://www.libtiff.org
//!
//! If libtiff is not present, tiffs are substituted by PNG's
//! Layer -1
//!

class Image2D {
	//! \todo: collect common flags from image2d.h and meshio.h (_READ_..., etc.)
	
	public:
		// constructor and deconstructor:
		Image2D();
		~Image2D() = default;

		void setResolution(const double xRes, const double yRes, const short resolutionUnit=0 );

		int writeTIFF(const std::string& filename, uint32_t width, uint32_t height, double* raster, double maxVal, bool isRGB=true );
		int writeTIFF(const std::string& filename, uint32_t width, uint32_t height, float*  raster, float  minVal=_NOT_A_NUMBER_, float  maxVal=_NOT_A_NUMBER_, bool isRGB=true );
		int writeTIFF(const std::string& filename, uint32_t width, uint32_t height, double* raster, double minVal=_NOT_A_NUMBER_, double maxVal=_NOT_A_NUMBER_, bool isRGB=true );
		//int writeTIFF(      std::string  filename, uint32_t width, uint32_t height, unsigned char* raster, bool isRGB=true );
		int writeTIFF(std::string filename, uint32_t width, uint32_t height, unsigned char* raster, bool isRGB=true );
		int writeTIFFStack(const std::string& filename, uint32_t width, uint32_t height, uint32_t stackheight, unsigned char* imageStack, bool isRGB=true );

	private:
		short  resolutionUnit; //!< None, Inch (DPI), Centimeter
		double xRes;           //!< dots per resolutionUnit in x
		double yRes;           //!< dots per resolutionUnit in y
};

#endif
