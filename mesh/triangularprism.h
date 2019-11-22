#ifndef TRIANGULARPRISM_H
#define TRIANGULARPRISM_H

#include <memory>

#include "vector3d.h"

class Plane;
class Line;

class TriangularPrism {
	public:
		TriangularPrism(Vector3D* vertices);
		Line gettriangularPrismEdge(int nr);
		std::unique_ptr<Plane> gettriangularPrismPlane(int nr);
		bool PointisintringularPrism(Vector3D& p);

	private:
		Vector3D mVertex[6];

};

#endif // TRIANGULARPRISM_H
