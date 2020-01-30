#ifndef BOXRECT_H
#define BOXRECT_H

#include "primitive.h"

//!
//! \brief Class for handling rectangular boxes. (Layer 0.5 - Datums)
//!
//! Rectangular Box (Datum) class.
//!
//!  
//!   F---G
//!  /   /|     |           N3
//!  B--C H     E--         |
//!  |  |/     /     Center O--> N2
//!  A--D                  /
//!                       N1
//! Layer 0.5
//!

class RectBox : public Primitive {

	public:
		// const- & destructor:
		RectBox( Vector3D vecCenter, Vector3D vec1, Vector3D vec2, Vector3D vec3 );
		~RectBox();

		// return the boxes center:
		double getX();
		double getY();
		double getZ();
		Vector3D getCoordCenter();

		int getType();

		// return the boxes corners:
		Vector3D getCoordA();
		Vector3D getCoordB();
		Vector3D getCoordC();
		Vector3D getCoordD();
		Vector3D getCoordE();
		Vector3D getCoordF();
		Vector3D getCoordG();
		Vector3D getCoordH();

	private:
		double *x;          //!< Reference/pointer for the x-coordinate to an array handling coordinates.
		double *y;          //!< Reference/pointer for the y-coordinate to an array handling coordinates.
		double *z;          //!< Reference/pointer for the z-coordinate to an array handling coordinates.
		// Non-normalized normal vectors. The length of the normal vector is half of the boxes edge length.
		double *nX1;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nY1;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nZ1;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nX2;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nY2;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nZ2;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nX3;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nY3;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		double *nZ3;         //!< Reference/pointer to an array handling the RectBoxes normal vectors.
		bool freePointers; //! In case we use the non-reference way of contructing, this will be true to remember to free allocated memory.
};

#endif
