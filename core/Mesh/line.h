#ifndef LINE_H
#define LINE_H

#include "vertex.h"

class Vertex;
class Plane;

class Line : public Primitive {

public:

	Line() = default;
	Line( const Vector3D& rPosA, const Vector3D& rPosB );
	Line( Vertex& rPosA, Vertex& rPosB );


	bool operator==(Line& l) const;


	// Information retrieval:

	void getA(Vector3D& p) const;
	void getB(Vector3D& p) const;
	Vector3D getA() const;
	Vector3D getB() const;
	void getA(Vertex& p) const;
	void getB(Vertex& p) const;
	void getData(Vector3D& a, Vector3D& b) const;
	double getLength() const;
	bool getDistanceToPoint(Vector3D &p, double &dist) const;
	bool getDistanceToPoint(Vector3D &p, double &dist, Line& shortest) const;


	// Operations:

	bool setA(Vector3D& p);
	bool setB(Vector3D& p);
	bool setA(Vertex& p);
	bool setB(Vertex& p);
	void setData(Vector3D& a, Vector3D& b);

	bool getLineIntersection(Line& other, Line& result) const;

	int getLinePlaneIntersection(Face& F, Vector3D& res);
	int getLinePlaneIntersection(Plane& P, Vector3D& res);

	void projectonPlane(Plane& tar, Line& res);


	// get t from equation p = a + t *(b-a)
	bool getParameter(Vector3D& p, double& t);


//	bool projectonPlane(Plane& tar, Line& res);

private:
	// Vertices
	Vertex* mVertA;            //!< Vertex A
	Vertex* mVertB;            //!< Vertex B

	Vector3D mVecA;            //!< Vector3D A
	Vector3D mVecB;            //!< Vector3D B

};

#endif // LINE_H
