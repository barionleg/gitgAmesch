/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPFEATUREVECS_H
#define COMPFEATUREVECS_H
#include <GigaMesh/mesh/mesh.h>

struct sMeshDataStruct {
	int     threadID{0}; //!< ID of the posix thread
	// input:
	Mesh*   meshToAnalyze{nullptr};
	double  radius{0.0};
	uint    xyzDim{0};
	uint    multiscaleRadiiSize{0};
	double* multiscaleRadii{nullptr};
	// output:
	int     ctrIgnored{0};
	int     ctrProcessed{0};
	// our most precious feature vectors (as array):
	double* patchNormal{nullptr};     //!< Normal used for orientation into 2.5D representation
	double* descriptVolume{nullptr};  //!< Volume descriptors
	double* descriptSurface{nullptr}; //!< Surface descriptors
	// and the voxel filter
	voxelFilter2DElements** sparseFilters{nullptr};
};

//! Compute the Multi-Scale Integral Invariant feature vectors
void compFeatureVectors(
        sMeshDataStruct*   rMeshData,
        const size_t       rThreadOffset,
        const size_t       rThreadVertexCount
);

#endif
