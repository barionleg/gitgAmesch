#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <stdlib.h>
#include <float.h>

#include "vertexofface.h"
#include "face.h"

using namespace std;

//! Main routine for testing surface area integral invariant
//==========================================================================
int main(void) {

	Vector3D aa(0.0, 0.0, 0.0);
	Vector3D bb(1.0, 0.0, 0.0);
	Vector3D cc(0.0, 1.0, 0.0);
	VertexOfFace a(aa);
	VertexOfFace b(bb);
	VertexOfFace c(cc);

	Face face1(1000000000, &a, &b, &c );
	Vector3D mm(0.0, 0.0, 0.0);
	VertexOfFace m(mm);
	double r=1.0;
	double area=0.0;
	//a.dumpInfo();
	//face1.dumpFaceInfo();
	face1.surfaceintegralinvariant(1, &r, &area, &m);


	cout<<"Area of intersection: "<<area<<endl;

	return 0;
}
