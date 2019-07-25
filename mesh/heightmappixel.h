#ifndef HEIGHTMAPPIXEL_H
#define HEIGHTMAPPIXEL_H

//!
//! \brief Structure for pixels of a height map.
//!
//! The struct holds the pixel-coordinates of the output image and the 
//! height-values.
//!
typedef unsigned int uint;

struct HeightMapPixel {
	uint  x; //!< x in image coordinate system.
	uint  y; //!< y in image coordinate system.
	float z; //!< z in Mesh coordiante system.
};

#endif
