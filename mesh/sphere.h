#ifndef SPHERE_H
#define SPHERE_H

#include "vertex.h"

//!
//! \brief Class for handling spheres. (Layer 0.5 - Datums)
//!
//! Sphere (Datum) class.
//!
//! Layer 0.5
//!

class Sphere : public Vertex {
	public:
		// const- & destructor:
		Sphere( Vector3D vertPos, float setRadius, unsigned char setR=0, unsigned char setG=0, unsigned char setB=0 );
		~Sphere();

		float getRadius();
		int   getType();

	private:
		float radius;       //!< Radius of the sphere.
};

#endif
