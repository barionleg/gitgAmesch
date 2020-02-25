#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <GigaMesh/mesh/vector3d.h>
#include <vector>

namespace GigaMesh {
    namespace Util {

	    //! Triangulates a simple polygon. The polygon has to be planar without selfintersections
	    //! @param vertices The vertices of the n-gon.
	    //! @return A vector of triangle indices. Returning size is 3 * (vertices.size() - 2)
	    std::vector<size_t> triangulateNgon(const std::vector<Vector3D>& vertices);
	}
}


#endif // TRIANGULATION_H
