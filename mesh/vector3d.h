#ifndef VECTOR3D_H
#define VECTOR3D_H


#include <iostream>
#include <sstream> // ostringstream

#include <cmath>
#include <cfloat> // FLT_MAX, FLT_MIN, FLT_EPS, etc.

//#include "matrix4d.h"

class Matrix4D;

//! \file vector3d.h
//!
//! \brief Simple 3D Vector for basic operations.
//!
//! Simple 3D Vector for basic operations.
//!

//! \class Vector3D
//!
//! \brief Simple 3D Vector for basic operations.
//!
//! Simple 3D Vector for basic operations.
//!

class Vector3D {

	public:
		Vector3D();
		Vector3D( double scalar );
		Vector3D( double setX, double setY, double setZ, double setH=1.0 );
		Vector3D( Vector3D* vecToCopy );
		Vector3D(const double* coordXYZH );
		Vector3D(const double* coordXYZ, double setH );
		Vector3D( double rPhi, double rTheta, bool rIsRadiant );

		void set( double setX, double setY, double setZ, double setH=1.0 );
		void set( Vector3D* vecToCopy );
		void set( Vector3D vecToCopy );
		void setX( double setX );
		void setY( double setY );
		void setZ( double setZ );
		void setH( double setH );

		// Access
		double getX() const;
		double getY() const;
		double getZ() const;
		double getH() const;
		double getLength3() const;
		double getLength4() const;
		double getAngleToXinXY();
		double getAngleToZinYZ();
		double getAngleToZ();
		bool  get( double* getX, double* getY, double* getZ, double* getH=nullptr );
		bool  get3( double* getXYZ );
		bool  get3( float* rPositionXYZ );
		bool  getString3( std::string* rStr );
		// Spherical coordinates:
		double getSphPhiDeg();
		double getSphThetaDeg();
		double getSphRadius();

		// Manipulation & Computation
		double normalize3();
		double setLength3( double newLen );
		double normalizeW();
		void   pow3( double exponent );
		void   pow4( double exponent );
		double sum3();
		double sum4();
		bool   projectOntoPlane( Vector3D rPlaneHNF );
		double distanceToLine( const Vector3D* rPos1, const Vector3D* rPos2 );
		double angleInLineCoord( const Vector3D* rPosTop, const Vector3D* rPosBottom );

		// Operators
		void operator += (Vector3D addVec);
		void operator -= (Vector3D addVec);
		void operator /= (double divVec);
		void operator *= (double multVec);
		void operator *= (Matrix4D multMat);
		bool operator== (Vector3D compVec);
		bool operator!= (Vector3D compVec);

		// Operations
		Vector3D  projectOnto(Vector3D someVec);
		Vector3D  projectOntoLine( const Vector3D& rVertA, const Vector3D& rVertB );
		Vector3D& applyTransformation(Matrix4D& transformationMatrix);

		// Debuging
		void dumpInfo( bool forMatlab=false, char* someNameTag=NULL );

	private:
		double x; //!< x-coordinate
		double y; //!< y-coordinate
		double z; //!< z-coordinate
		double h; //!< homogenous coordinate
};

// GLOBAL Operators ------------------------------------------------------------

Vector3D  operator% ( Vector3D crossVec1, Vector3D crossVec2 );
Vector3D  operator+ ( Vector3D someVec1, Vector3D someVec2 );
Vector3D  operator- ( Vector3D someVec );
Vector3D  operator- ( Vector3D someVec1, Vector3D someVec2 );
double    operator* ( Vector3D someVec1, Vector3D someVec2 );
Vector3D  operator* ( Vector3D someVec, Matrix4D someMat );
Vector3D  operator* ( Matrix4D someMat, Vector3D someVec );
Vector3D  operator* ( Vector3D someVec, double scalar );
Vector3D  operator* ( double scalar, Vector3D someVec );
Vector3D  operator/ ( Vector3D someVec, double scalar );
Vector3D  operator/ ( double scalar, Vector3D someVec );
Vector3D  normalize3( Vector3D someVec );
Vector3D  normalize4( Vector3D someVec );

Vector3D  compMult( Vector3D someVec1, Vector3D someVec2 );

double     dot3( Vector3D someVec1, Vector3D someVec2 );
double     dot4( Vector3D someVec1, Vector3D someVec2 );
double     angle( Vector3D vec1, Vector3D vec2 );
double     angle( Vector3D vec1, Vector3D vec2, Vector3D vecNormal );
double     angle3( double* vec1, double* vec2 );
double     angle3ToZ( double* vec );
double     abs3( Vector3D someVec );
double     abs4( Vector3D someVec );

bool lineLineIntersect( const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, const Vector3D& p4, Vector3D& pa, Vector3D& pb );

std::ostream& operator<<(std::ostream& o, const Vector3D& v);

#endif
