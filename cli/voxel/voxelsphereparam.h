#ifndef VOXELSPHEREPARAM_H
#define VOXELSPHEREPARAM_H

#include <stdlib.h> // uint

//!
//! \brief Structure to set parameters for estimation of a Sphere described by voxels.
//!
//! The struct holds the diameter, density, raster mode, etc.
//!
typedef unsigned int uint;

struct VoxelSphereParam {
	uint  sphereRadius; //!< Radius of the sphere in voxel.
	float baseDensity;  //!< Density offset multiplied with whatever density function used (use 1.0 for full voxels). 
	int   rasterMode;   //!< Defines the operation with existing data in the grid.
	int   radiusFunc;   //!< Defines the function used to estimate a voxels density as function of the radius.
	float gaussFuncSig; //!< Parameter sigma of the gaussian function (use 0.4 for a start as it will scale the density to ~maximum at the center).
	float gaussFuncMu;  //!< Parameter mu (µ) of the gaussian function (use 0.0 for a start as it will scale the density to ~0 at the spheres surface).
};

#endif
