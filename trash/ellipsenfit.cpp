#include "ellipsenfit.h"

// use "Bookstein" or "FPF" (=Fitzgibbon, Pilu, Fisher) algorithm for quadratic fit to more than five points
// "conic" performs linear fit to exactly five points

#define BOOKSTEIN      1
#define FPF            2
#define CONIC          3

using namespace std;


#define FLOAT double

#define cross(a, b, ab) ab[0] = a[1]*b[2] - a[2]*b[1]; \
	ab[1] = a[2]*b[0] - a[0]*b[2]; \
	ab[2] = a[0]*b[1] - a[1]*b[0];

#define abs(x) ((x > 0.0) ? x : (-x))

//! Fake-Constructor for Ellipse-Object
Ellipsenfit::Ellipsenfit() {
}

//! Constructor for Ellipse-Object
//! \param faceList list of all triangles in the mesh
//! \param schnittebenenNormale vector for orientation of HNF
//! \param centerOfBoundingBox vector for position of HNF
Ellipsenfit::Ellipsenfit(set<Face*> faceList, Vector3D schnittebenenNormale, Vector3D centerOfBoundingBox) { //, Vector3D toCenter):Mesh() {

	Vector3D Y;
	Y.setX(0);
	Y.setY(1);
	Y.setZ(0);
	Y.setH(1);

	// rotationAxis of cuttingPlane
	rotationsAchse.set(0,0,1,0);
	if (angle(Y,schnittebenenNormale)>0)  rotationsAchse = Y % schnittebenenNormale;

	angleToY = angle(Y , schnittebenenNormale);

	// "Hesse normal form" is used to find cuts in the mesh --> http://en.wikipedia.org/wiki/Hesse_normal_form
	HNF_normale = schnittebenenNormale;
	HNF_position = centerOfBoundingBox;

	// translation to origin and inverse operation
	Matrix4D operationTranslate(-HNF_position.getX(),-HNF_position.getY(), -HNF_position.getZ());
	Matrix4D operationTranslateBack(HNF_position.getX(),HNF_position.getY(), HNF_position.getZ());

	rotationsAchse.normalize3();
	// rotation about arbitrary axis --> http://www.cprogramming.com/tutorial/3d/rotation.html (left)
	Matrix4D operationRotate(_MATRIX4D_INIT_IDENTITY_, 1);
	operationRotate.set(0,0,(1-cos (angleToY))*rotationsAchse.getX()*rotationsAchse.getX() + cos(angleToY));
	operationRotate.set(0,1,(1-cos (angleToY))*rotationsAchse.getX()*rotationsAchse.getY() - sin(angleToY)*rotationsAchse.getZ());
	operationRotate.set(0,2,(1-cos (angleToY))*rotationsAchse.getX()*rotationsAchse.getZ() + sin(angleToY)*rotationsAchse.getY());
	operationRotate.set(1,0,(1-cos (angleToY))*rotationsAchse.getX()*rotationsAchse.getY() + sin(angleToY)*rotationsAchse.getZ());
	operationRotate.set(1,1,(1-cos (angleToY))*rotationsAchse.getY()*rotationsAchse.getY() + cos(angleToY));
	operationRotate.set(1,2,(1-cos (angleToY))*rotationsAchse.getY()*rotationsAchse.getZ() - sin(angleToY)*rotationsAchse.getX());
	operationRotate.set(2,0,(1-cos (angleToY))*rotationsAchse.getX()*rotationsAchse.getZ() - sin(angleToY)*rotationsAchse.getY());
	operationRotate.set(2,1,(1-cos (angleToY))*rotationsAchse.getY()*rotationsAchse.getZ() + sin(angleToY)*rotationsAchse.getX());
	operationRotate.set(2,2,(1-cos (angleToY))*rotationsAchse.getZ()*rotationsAchse.getZ() + cos(angleToY));

	// ... and invers operation
	Matrix4D operationRotateBack(_MATRIX4D_INIT_IDENTITY_, 1);
	operationRotateBack.set(0,0,(1-cos (-angleToY))*rotationsAchse.getX()*rotationsAchse.getX() + cos(-angleToY));
	operationRotateBack.set(0,1,(1-cos (-angleToY))*rotationsAchse.getX()*rotationsAchse.getY() - sin(-angleToY)*rotationsAchse.getZ());
	operationRotateBack.set(0,2,(1-cos (-angleToY))*rotationsAchse.getX()*rotationsAchse.getZ() + sin(-angleToY)*rotationsAchse.getY());
	operationRotateBack.set(1,0,(1-cos (-angleToY))*rotationsAchse.getX()*rotationsAchse.getY() + sin(-angleToY)*rotationsAchse.getZ());
	operationRotateBack.set(1,1,(1-cos (-angleToY))*rotationsAchse.getY()*rotationsAchse.getY() + cos(-angleToY));
	operationRotateBack.set(1,2,(1-cos (-angleToY))*rotationsAchse.getY()*rotationsAchse.getZ() - sin(-angleToY)*rotationsAchse.getX());
	operationRotateBack.set(2,0,(1-cos (-angleToY))*rotationsAchse.getX()*rotationsAchse.getZ() - sin(-angleToY)*rotationsAchse.getY());
	operationRotateBack.set(2,1,(1-cos (-angleToY))*rotationsAchse.getY()*rotationsAchse.getZ() + sin(-angleToY)*rotationsAchse.getX());
	operationRotateBack.set(2,2,(1-cos (-angleToY))*rotationsAchse.getZ()*rotationsAchse.getZ() + cos(-angleToY));


	operationFull = operationTranslate;
	operationFull *= operationRotate;
	operationInverse = operationRotateBack;
	operationInverse *= operationTranslateBack;

	// find cuttingPoints and store them in global vector "Schnittpunkte"
	findCuttingPoints(faceList);

}

//! Destructor for Ellipse-Object
Ellipsenfit::~Ellipsenfit() {
}

//! chooses points from the global "Schnittpunkte"-vector and tries to fit an ellipse to them
//! \return ellipse parameter
vector<double> Ellipsenfit::getEllipseParameter() {
	// temp store for parameter
	vector<double> eParams;

	// indicates bad set of parameters
	eParams.push_back(-1.0);

	// temp store for averaging parameters
	double par0=0, par1=0, par2=0, par3=0, par4=0, par5=0;
	vector<vector<double> > manyParams;

	// only the 3D centerPoints of the found ellipses --> (x=parameter1 , 0, z=parameter2)*operationInverse
	vector<Vector3D> manyAttemps;

	// mean parameters which are returned as solution for this fit
	vector<double> ellipsenParameter;

	double circularity = 1000.0;

	int inWhile=0;
	size_t count;

	// --> collect
	while (manyAttemps.size()<50 && inWhile<50) {
		srand(time(NULL));

		//! 1) try linear fit with different sets of five randomly selected cutPoints (fast approach)
		count = 0;
		while (eParams[0]<0 && count < 100){
			count++;
			eParams = findEllipseParams(CONIC,reduceNumberOfPointsRandomly(5));
			if (eParams[0]>=0) circularity = testCircularity(eParams[3], eParams[4]);
		}

		//! 2) try quadratic fit by FPF / increasing number of equidistant selected cutPoints (slow but optimized for ellipses)
		count = 6;
		while (eParams[0]<0 && count+5 < Schnittpunkte.size()) {
			eParams = findEllipseParams(FPF, reduceNumberOfPoints(count=count+5));
			if (eParams[0]>=0) circularity = testCircularity(eParams[3], eParams[4]);
		}

		//! 3) try quadratic fit by BOOKSTEIN / increasing number of equidistant selected cutPoints (slow, robust, poor results)
		count = 6;
		while (eParams[0]<0 && count+5 < Schnittpunkte.size()) {
			eParams = findEllipseParams(BOOKSTEIN, reduceNumberOfPoints(count=count+5));
			if (eParams[0]>=0) circularity = testCircularity(eParams[3], eParams[4]);
		}

		//! 4) success (par0>=0) --> store center and aggregate params
		if (eParams[0]>=0) {
			inWhile=0;
			manyAttemps.push_back( Vector3D( (float)eParams[1], 0.0f, (float)eParams[2] )*operationInverse );
			par0+=circularity, par1+=eParams[1], par2+=eParams[2], par3+=eParams[3], par4+=eParams[4], par5+=eParams[5];
			manyParams.push_back(eParams);
		}
		else
			inWhile++;
	}

	if (!manyAttemps.empty()) {
		size_t count = manyAttemps.size();

		// Eliminate some of the worst results of all stored
		// currently works properly for points, but not for sets of parameters

		//		// delete outlier --> Points (working)
		//		while (manyAttemps.size()>40)
		//			manyAttemps = deleteOutlier(manyAttemps);

		//		// delete outlier --> Params (not working)
		//		while (manyPrams.size()>40)
		//			manyParams = deleteOutlier(manyParams);

		//! 5) average found parameters and return result
		//! par0 < 0 indicates corrupted parameter
		//! par1 & par2 = x-z-coordinate of center
		//! par3 & par4 = radius r1 & r2 of ellipse
		//! par5 = theta-angle of ellipseRotation
		par0/=count, par1/=count, par2/=count, par3/=count, par4/=count, par5/=count;
		ellipsenParameter.push_back(par0);
		ellipsenParameter.push_back(par1);
		ellipsenParameter.push_back(par2);
		ellipsenParameter.push_back(par3);
		ellipsenParameter.push_back(par4);
		ellipsenParameter.push_back(par5);
	}

	return ellipsenParameter;
}

//! calculates the circularity of the ellipse
//! \param r1 radius1
//! \param r2 radius2
//! \return 1.0 for a cycle to 1000.0 for a useless "thing"
double Ellipsenfit::testCircularity(double r1, double r2) {
	double circularity = 1000.0;
	if (r1 == 0 || r2 == 0) return circularity;
	if (r1>=r2)  circularity = r1/r2;
	if (r1<r2)  circularity = r2/r1;
	return circularity;
}

//! calculates the mean center in a cloud if points
//! \param many points
//! \return average center of given points
Vector3D Ellipsenfit::averageCenter(vector<Vector3D> points) {
	Vector3D average;
	for (size_t i = 0; i < points.size(); ++i)
		average += points[i];
	average /= points.size();
	return average;
}

//! Selects a number of points from the global "Schnittpunkte"-vector in equidistant stept through the vector
//! \param how many points to choose
//! \return subset of points
vector<Vector3D> Ellipsenfit::reduceNumberOfPoints(size_t pointsCount) {
	if (Schnittpunkte.size()<pointsCount) return Schnittpunkte;
	vector<Vector3D> subset;

	// Menge der zu fittenden Punkte verringern
	for(size_t i = 0;i < Schnittpunkte.size();i++)
		if (Schnittpunkte.size() <6 || i % ((Schnittpunkte.size()/pointsCount))==0) { // equidistante schritte
			subset.push_back(Schnittpunkte[i]*operationFull);
		}
	return subset;
}

//! Randomly selects a number of (different) points from the global "Schnittpunkte"-vector
//! \param how many points to choose
//! \return subset of points
vector<Vector3D> Ellipsenfit::reduceNumberOfPointsRandomly(size_t pointsCount) {
	if (Schnittpunkte.size()<pointsCount) return Schnittpunkte;
	vector<Vector3D> subset;
	vector<int> randomNumbers;

	while(pointsCount>randomNumbers.size()) {
		int randNum = rand()%Schnittpunkte.size();
		bool isAlreadyThere = false;
		for (size_t i = 0;i< randomNumbers.size(); i++)
			if(randomNumbers[i] == randNum)
				isAlreadyThere = true;
		if(!isAlreadyThere)
			randomNumbers.push_back(randNum);
	}
	for (size_t i = 0; i < randomNumbers.size(); ++i) {
		subset.push_back(Schnittpunkte[randomNumbers[i]]*operationFull);
	}
	return subset;
}

//! computes the averageCenter of a given set of points and deletes the most far point
//! \param points (preferably more than two)
//! \return all given points except the worst outlier
vector<Vector3D> Ellipsenfit::deleteOutlier(vector<Vector3D> points) {

	// points.size() == 0 results in segmentation fault
	// points.size() == 1 results in empty return-vector
	// points.size() == 2 result is useless

	if (points.size()<3) return points;


	vector<Vector3D> average;
	vector<Vector3D> cleanedPoints;
	Vector3D averagedCenter = averageCenter(points);
	Vector3D toDelete = points[0];

	for (size_t i = 0; i < points.size(); ++i)
		if ((points[i]-averagedCenter).getLength3() > (toDelete-averagedCenter).getLength3()) toDelete = points[i];

	for (size_t i = 0; i < points.size(); ++i)
		if (points[i] != toDelete) average.push_back(points[i]);

	return average;
}

//! computes the mean ellipseParameters from a given set of ellipses and deletes the worst outlier
//! \param parameters of at least three different ellipses
//! \return all given ellipses except the worst outlier
vector<vector<double> > Ellipsenfit::deleteOutlier(vector<vector<double> > params) {

	// params.size() == 0 results in segmentation fault
	// params.size() == 1 results in empty return-vector
	// params.size() == 2 result is useless

	vector<Vector3D> points;

	for (size_t i = 0; i<params.size();i++) {
		points.push_back( Vector3D( (float)params[i][1], 0.0f, (float)params[i][2] ) );
	}

	if (points.size()<3) return params;


	vector<vector<double> > average;
	Vector3D averagedCenter = averageCenter(points);
	Vector3D toDelete = points[0];

	for (size_t i = 0; i < points.size(); ++i)
		if ((points[i]-averagedCenter).getLength3() > (toDelete-averagedCenter).getLength3()) toDelete = points[i];

	for (size_t i = 0; i < points.size(); ++i)
		if (points[i] != toDelete) average.push_back(params[i]);

	return average;
}

//! checks all faces in the given faceList to find cuts with the current cuttingPlaneSetup
//! \param list of faces to check
void Ellipsenfit::findCuttingPoints(set<Face*> faceList) {
	set<Face*>::iterator it;
	// precomputed lambda for HNF-Distance
	float lambda = dot3(HNF_normale, HNF_position);

	// face is ignored if first point returns a distance greater than this one
	float ignoreDistance = 1;

	//! stores points found in the global Schnittpunkte-vector if they are not already there (cut at edge or vertex insteat of center)
	for( it=faceList.begin(); it!=faceList.end(); it++ ) {
		Vertex *A = (*it)->getVertA();
		float distA = dot3(A->getPositionVector(), HNF_normale)-lambda;
		if (distA > ignoreDistance) continue;
		Vertex *B = (*it)->getVertB();
		float distB = dot3(B->getPositionVector(), HNF_normale)-lambda;

		Vertex *C = (*it)->getVertC();
		float distC = dot3(C->getPositionVector(), HNF_normale)-lambda;


		// no hit
		if ((distA<0 && distB<0 && distC<0) || (distA>0 && distB>0 && distC>0))
			continue;

		// edge Hit (double hits for two triangles !!!)
		if (distA == 0 && distB == 0) {
			Vector3D theNewOne = (A->getPositionVector() + B->getPositionVector()) / 2;
			if (!isPointInVector(Schnittpunkte, theNewOne))
				Schnittpunkte.push_back(theNewOne);
			continue;
		}

		if (distA == 0 && distC == 0) {
			Vector3D theNewOne = (A->getPositionVector() + C->getPositionVector()) / 2;
			if (!isPointInVector(Schnittpunkte, theNewOne))
				Schnittpunkte.push_back(theNewOne);
			continue;
		}

		if (distC == 0 && distB == 0) {
			Vector3D theNewOne = (C->getPositionVector() + B->getPositionVector()) / 2;
			if (!isPointInVector(Schnittpunkte, theNewOne))
				Schnittpunkte.push_back(theNewOne);
			continue;
		}

		// vertex hit (triple hits for all three triangles !!!)
		if (distA == 0) {
			if (!isPointInVector(Schnittpunkte, A->getPositionVector()))
				Schnittpunkte.push_back(A->getPositionVector());
			continue;
		}

		if (distB == 0) {
			if (!isPointInVector(Schnittpunkte, B->getPositionVector()))
				Schnittpunkte.push_back(B->getPositionVector());
			continue;
		}

		if (distC == 0) {
			if (!isPointInVector(Schnittpunkte, C->getPositionVector()))
				Schnittpunkte.push_back(C->getPositionVector());
			continue;
		}

		// central hit
		if ((distA < 0 && distB < 0 && distC > 0) || (distA > 0 && distB > 0 && distC < 0)) {
			Schnittpunkte.push_back(findEdgeCut((A->getPositionVector() + B->getPositionVector()) / 2, C->getPositionVector()));
			continue;
		}
		if ((distA < 0 && distB > 0 && distC > 0) || (distA > 0 && distB < 0 && distC < 0)) {
			Schnittpunkte.push_back(findEdgeCut((C->getPositionVector() + B->getPositionVector()) / 2, A->getPositionVector()));
			continue;
		}
		if ((distA > 0 && distB < 0 && distC > 0) || (distA < 0 && distB > 0 && distC < 0)) {
			Schnittpunkte.push_back(findEdgeCut((A->getPositionVector() + C->getPositionVector()) / 2, B->getPositionVector()));
			continue;
		}
	}
}

//! checks if a given point is already contained in a given vector of points
//! \param theVector vector to look for the point in
//! \param thePoint point to look for
//! \return true if the point is found in the vector
bool Ellipsenfit::isPointInVector(vector<Vector3D> theVector, Vector3D thePoint) {
	bool alreadyContainsPoint = false;
	for (size_t i = 0; i<theVector.size();i++)
		if (theVector[i].getX() == thePoint.getX() && theVector[i].getY() == thePoint.getY() && theVector[i].getZ() == thePoint.getZ()) {
			alreadyContainsPoint = true;
			break;
		}
	return alreadyContainsPoint;
}

//! checks if the given points define an edge cutting the current cuttingPlaneSetup
//! \param A first point of edge
//! \param B second point of edge
//! \return true if the edge cuts the plane
bool Ellipsenfit::edgeCheck(Vector3D A, Vector3D B) {
	bool cuts = false;
	float lambda = dot3(HNF_normale, HNF_position);
	if (((dot3(A , HNF_normale)-lambda) * (dot3(B , HNF_normale)-lambda)) < 0) cuts = true;
	return cuts;
}

//! calculates the cuttingPoint between the given points definimg one edge and the current cuttingPlaneSetup
//! \param A first point of edge
//! \param B second point of edge
//! \return cuttingPoint
Vector3D Ellipsenfit::findEdgeCut(Vector3D edgeVertA, Vector3D edgeVertB) {
	float lambda = dot3(HNF_normale , HNF_position);
	Vector3D edgeDirection;
	edgeDirection = edgeVertA - edgeVertB;

	float la = 0;
	la = (lambda - dot3(HNF_normale , edgeVertA)) / dot3(HNF_normale , edgeDirection);

	Vector3D schnittpunkt;
	schnittpunkt = edgeVertA + la * edgeDirection;
	schnittpunkt.setH(1);
	return schnittpunkt;
}

/* toconic takes five points in homogeneous coordinates, and returns the
 * coefficients of a general conic equation in a, b, c, ..., f:
 *
 * a*x*x + b*x*y + c*y*y + d*x + e*y + f = 0.
 *
 * The routine returns 1 on success; 0 otherwise.  (It can fail, for
 * example, if there are duplicate points.
 *
 * Typically, the points will be finite, in which case the third (w)
 * coordinate for all the input vectors will be 1, although the code
 * deals cleanly with points at infinity.
 *
 * For example, to find the equation of the conic passing through (5, 0),
 * (-5, 0), (3, 2), (3, -2), and (-3, 2), set:
 *
 * p0[0] =  5, p0[1] =  0, p0[2] = 1,
 * p1[0] = -5, p1[1] =  0, p1[2] = 1,
 * p2[0] =  3, p2[1] =  2, p2[2] = 1,
 * p3[0] =  3, p3[1] = -2, p3[2] = 1,
 * p4[0] = -3, p4[1] =  2, p4[2] = 1.
 *
 * But if you want the equation of the hyperbola that is tangent to the
 * line 2x=y at infinity,  simply make one of the points be the point at
 * infinity along that line, for example:
 *
 * p0[0] = 1, p0[1] = 2, p0[2] = 0.
 */


//! checks if the 3D centerPoints define an axis orthogonal to the cuttingPlanes in proper distance.
//! \param exactly five points to fit an ellipse to
//! \return error, centerX, centerY, radius1, radius2, theta
vector<double> Ellipsenfit::toconic(vector<Vector3D> subset) {
	vector<double> params;
	vector<double> eparams;
	eparams.push_back(-1.0);
	if (subset.empty()) {
		return eparams;
	}
	FLOAT p0[3];
	p0[0] = subset[0].getX();
	p0[1] = subset[0].getZ();
	p0[2] = 1;

	FLOAT p1[3];
	p1[0] = subset[1].getX();
	p1[1] = subset[1].getZ();
	p1[2] = 1;

	FLOAT p2[3];
	p2[0] = subset[2].getX();
	p2[1] = subset[2].getZ();
	p2[2] = 1;

	FLOAT p3[3];
	p3[0] = subset[3].getX();
	p3[1] = subset[3].getZ();
	p3[2] = 1;

	FLOAT p4[3];
	p4[0] = subset[4].getX();
	p4[1] = subset[4].getZ();
	p4[2] = 1;

	FLOAT L0[3], L1[3], L2[3], L3[3];
	FLOAT A, B, C, Q[3];
	FLOAT a1, a2, b1, b2, c1, c2;
	FLOAT x0, x4, y0, y4, w0, w4;
	FLOAT aa, bb, cc, dd, ee, ff;
	FLOAT y4w0, w4y0, w4w0, y4y0, x4w0, w4x0, x4x0, y4x0, x4y0;
	FLOAT a1a2, a1b2, a1c2, b1a2, b1b2, b1c2, c1a2, c1b2, c1c2;

	cross(p0, p1, L0)
	cross(p1, p2, L1)
	cross(p2, p3, L2)
	cross(p3, p4, L3)
	cross(L0, L3, Q)
	A = Q[0]; B = Q[1]; C = Q[2];
	a1 = L1[0]; b1 = L1[1]; c1 = L1[2];
	a2 = L2[0]; b2 = L2[1]; c2 = L2[2];
	x0 = p0[0]; y0 = p0[1]; w0 = p0[2];
	x4 = p4[0]; y4 = p4[1]; w4 = p4[2];

	y4w0 = y4*w0;
	w4y0 = w4*y0;
	w4w0 = w4*w0;
	y4y0 = y4*y0;
	x4w0 = x4*w0;
	w4x0 = w4*x0;
	x4x0 = x4*x0;
	y4x0 = y4*x0;
	x4y0 = x4*y0;
	a1a2 = a1*a2;
	a1b2 = a1*b2;
	a1c2 = a1*c2;
	b1a2 = b1*a2;
	b1b2 = b1*b2;
	b1c2 = b1*c2;
	c1a2 = c1*a2;
	c1b2 = c1*b2;
	c1c2 = c1*c2;

	aa = -A*a1a2*y4w0
	+A*a1a2*w4y0
	-B*b1a2*y4w0
	-B*c1a2*w4w0
	+B*a1b2*w4y0
	+B*a1c2*w4w0
	+C*b1a2*y4y0
	+C*c1a2*w4y0
	-C*a1b2*y4y0
	-C*a1c2*y4w0;

	cc =  A*c1b2*w4w0
	+A*a1b2*x4w0
	-A*b1c2*w4w0
	-A*b1a2*w4x0
	+B*b1b2*x4w0
	-B*b1b2*w4x0
	+C*b1c2*x4w0
	+C*b1a2*x4x0
	-C*c1b2*w4x0
	-C*a1b2*x4x0;

	ff =  A*c1a2*y4x0
	+A*c1b2*y4y0
	-A*a1c2*x4y0
	-A*b1c2*y4y0
	-B*c1a2*x4x0
	-B*c1b2*x4y0
	+B*a1c2*x4x0
	+B*b1c2*y4x0
	-C*c1c2*x4y0
	+C*c1c2*y4x0;

	bb =  A*c1a2*w4w0
	+A*a1a2*x4w0
	-A*a1b2*y4w0
	-A*a1c2*w4w0
	-A*a1a2*w4x0
	+A*b1a2*w4y0
	+B*b1a2*x4w0
	-B*b1b2*y4w0
	-B*c1b2*w4w0
	-B*a1b2*w4x0
	+B*b1b2*w4y0
	+B*b1c2*w4w0
	-C*b1c2*y4w0
	-C*b1a2*x4y0
	-C*b1a2*y4x0
	-C*c1a2*w4x0
	+C*c1b2*w4y0
	+C*a1b2*x4y0
	+C*a1b2*y4x0
	+C*a1c2*x4w0;

	dd = -A*c1a2*y4w0
	+A*a1a2*y4x0
	+A*a1b2*y4y0
	+A*a1c2*w4y0
	-A*a1a2*x4y0
	-A*b1a2*y4y0
	+B*b1a2*y4x0
	+B*c1a2*w4x0
	+B*c1a2*x4w0
	+B*c1b2*w4y0
	-B*a1b2*x4y0
	-B*a1c2*w4x0
	-B*a1c2*x4w0
	-B*b1c2*y4w0
	+C*b1c2*y4y0
	+C*c1c2*w4y0
	-C*c1a2*x4y0
	-C*c1b2*y4y0
	-C*c1c2*y4w0
	+C*a1c2*y4x0;

	ee = -A*c1a2*w4x0
	-A*c1b2*y4w0
	-A*c1b2*w4y0
	-A*a1b2*x4y0
	+A*a1c2*x4w0
	+A*b1c2*y4w0
	+A*b1c2*w4y0
	+A*b1a2*y4x0
	-B*b1a2*x4x0
	-B*b1b2*x4y0
	+B*c1b2*x4w0
	+B*a1b2*x4x0
	+B*b1b2*y4x0
	-B*b1c2*w4x0
	-C*b1c2*x4y0
	+C*c1c2*x4w0
	+C*c1a2*x4x0
	+C*c1b2*y4x0
	-C*c1c2*w4x0
	-C*a1c2*x4x0;

	if (aa != 0.0) {
		bb /= aa; cc /= aa; dd /= aa; ee /= aa; ff /= aa; aa = 1.0;
	} else if (bb != 0.0) {
		cc /= bb; dd /= bb; ee /= bb; ff /= bb; bb = 1.0;
	} else if (cc != 0.0) {
		dd /= cc; ee /= cc; ff /= cc; cc = 1.0;
	} else if (dd != 0.0) {
		ee /= dd; ff /= dd; dd = 1.0;
	} else if (ee != 0.0) {
		ff /= ee; ee = 1.0;
	}

	params.push_back(0.0);
	params.push_back(aa);
	params.push_back(bb);
	params.push_back(cc);
	params.push_back(dd);
	params.push_back(ee);
	params.push_back(ff);

	// polynom to standard parameter - not used
	//    FLOAT x, y, w;
	//    x = p0[0]; y = p0[1]; w = p0[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p1[0]; y = p1[1]; w = p1[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p2[0]; y = p2[1]; w = p2[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p3[0]; y = p3[1]; w = p3[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);
	//    x = p4[0]; y = p4[1]; w = p4[2];
	//    eparams.push_back(params[1]*x*x+params[2]*x*y+params[3]*y*y+params[4]*x*w+params[5]*y*w+params[6]*w*w);

	// polynom to standard parameter
	eparams = solveellipse(params);
	return eparams;
}

//FLOAT p0[3] = {0, 0, 1};
//FLOAT p1[3] = {1, 1, 1};
//FLOAT p2[3] = {-1, -1, 1};
//FLOAT p3[3] = {2, 2, 1};
//FLOAT p4[3] = {3, 3, 1};

//void main() {
//    FLOAT a, b, c, d, e, f, s0, s1, s2, s3, s4;
//    FLOAT x, y, w;
//    int i, ret;

/*
    ret = toconic(p0, p1, p2, p3, p4, &a, &b, &c, &d, &e, &f);
    if (ret == 1) {
	printf("success\n");
	printf("%g %g %g %g %g %g\n", a, b, c, d, e, f);
	x = p0[0]; y = p0[1]; w = p0[2];
	printf("%g ", a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w);
	x = p1[0]; y = p1[1]; w = p1[2];
	printf("%g ", a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w);
	x = p2[0]; y = p2[1]; w = p2[2];
	printf("%g ", a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w);
	x = p3[0]; y = p3[1]; w = p3[2];
	printf("%g ", a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w);
	x = p4[0]; y = p4[1]; w = p4[2];
	printf("%g\n", a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w);
    } else {
	printf("toconic failed\n");
    }
 */

//    for (i = 0; i < 100000; i++) {
//	p0[0] = (FLOAT) (rand()%30);
//	p0[1] = (FLOAT) (rand()%30);
//	p0[2] = (FLOAT) (rand()%30);
//	p1[0] = (FLOAT) (rand()%30);
//	p1[1] = (FLOAT) (rand()%30);
//	p1[2] = (FLOAT) (rand()%30);
//	p2[0] = (FLOAT) (rand()%30);
//	p2[1] = (FLOAT) (rand()%30);
//	p2[2] = (FLOAT) (rand()%30);
//	p3[0] = (FLOAT) (rand()%30);
//	p3[1] = (FLOAT) (rand()%30);
//	p3[2] = (FLOAT) (rand()%30);
//	p4[0] = (FLOAT) (rand()%30);
//	p4[1] = (FLOAT) (rand()%30);
//	p4[2] = (FLOAT) (rand()%30);
//	if (toconic(p0, p1, p2, p3, p4, &a, &b, &c, &d, &e, &f)) {
//	    x = p0[0]; y = p0[1]; w = p0[2];
//	    s0=a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w;
//	    x = p1[0]; y = p1[1]; w = p1[2];
//	    s1=a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w;
//	    x = p2[0]; y = p2[1]; w = p2[2];
//	    s2=a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w;
//	    x = p3[0]; y = p3[1]; w = p3[2];
//	    s3=a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w;
//	    x = p4[0]; y = p4[1]; w = p4[2];
//	    s4=a*x*x+b*x*y+c*y*y+d*x*w+e*y*w+f*w*w;
//	    if (abs(s0) > .00001 || abs(s1) > .00001 || abs(s2) > .00001
//		|| abs(s3) > .00001 || abs(s4) > .00001) {
//		    printf("%g %g %g %g %g\n", s0, s1, s2, s3, s4);
//		    printf("\t%g %g %g\n", p0[0], p0[1], p0[2]);
//		    printf("\t%g %g %g\n", p1[0], p1[1], p1[2]);
//		    printf("\t%g %g %g\n", p2[0], p2[1], p2[2]);
//		    printf("\t%g %g %g\n", p3[0], p3[1], p3[2]);
//		    printf("\t%g %g %g\n", p4[0], p4[1], p4[2]);
//		    printf("%g %g %g %g %g %g\n", a, b, c, d, e, f);
//		}
//	}
//    }
//}

//! special operation needed by jacobi-function
//! \param a the matrix we are operating on
//! \param i row of first element
//! \param j column of first element
//! \param k row of second element
//! \param l column of second element
//! \param tau set by jacobi-function
//! \param s set by jacobi-function
void Ellipsenfit::ROTATE(double** a, int i, int j, int k, int l, double tau, double s) {
	double g,h;
	g=a[i][j];
	h=a[k][l];
	a[i][j]=g-s*(h+g*tau);
	a[k][l]=h+s*(g-h*tau);
}

//! computes Jacobi-matrix
//! \param a the matrix we are operating on
//! \param n size of (NxN) marix
//! \param d array with eigenvalues
//! \param v martix with eigenvectors
//! \param nrot not longer used
void Ellipsenfit::jacobi(double** a, int n, double d[] , double** v, int nrot) {
	int j,iq,ip,i;
	double tresh,theta,tau,t,sm,s,h,g,c;

	double* b;
	b=(double*)malloc((n+1)*sizeof(double));
	double* z;
	z=(double*)malloc((n+1)*sizeof(double));

	for (ip=1;ip<=n;ip++) {
		for (iq=1;iq<=n;iq++) v[ip][iq]=0.0;
		v[ip][ip]=1.0;
	}
	for (ip=1;ip<=n;ip++) {
		b[ip]=d[ip]=a[ip][ip];
		z[ip]=0.0;
	}
	nrot=0;
	for (i=1;i<=50;i++) {
		sm=0.0;
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++)
				sm += abs(a[ip][iq]);
		}
		if (sm == 0.0) {
			/*    free_vector(z,1,n);
		  free_vector(b,1,n);  */
			return;
		}
		if (i < 4)
			tresh=0.2*sm/(n*n);
		else
			tresh=0.0;
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++) {
				g=100.0*abs(a[ip][iq]);
				if (i > 4 && abs(d[ip])+g == abs(d[ip])
						&& abs(d[iq])+g == abs(d[iq]))
					a[ip][iq]=0.0;
				else if (abs(a[ip][iq]) > tresh) {
					h=d[iq]-d[ip];
					if (abs(h)+g == abs(h))
						t=(a[ip][iq])/h;
					else {
						theta=0.5*h/(a[ip][iq]);
						t=1.0/(abs(theta)+sqrt(1.0+theta*theta));
						if (theta < 0.0) t = -t;
					}
					c=1.0/sqrt(1+t*t);
					s=t*c;
					tau=s/(1.0+c);
					h=t*a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip][iq]=0.0;
					for (j=1;j<=ip-1;j++) {
						ROTATE(a,j,ip,j,iq,tau,s);
					}
					for (j=ip+1;j<=iq-1;j++) {
						ROTATE(a,ip,j,j,iq,tau,s);
					}
					for (j=iq+1;j<=n;j++) {
						ROTATE(a,ip,j,iq,j,tau,s);
					}
					for (j=1;j<=n;j++) {
						ROTATE(v,j,ip,j,iq,tau,s);
					}
					++nrot;
				}
			}
		}
		for (ip=1;ip<=n;ip++) {
			b[ip] += z[ip];
			d[ip]=b[ip];
			z[ip]=0.0;
		}
	}
	//printf("Too many iterations in routine JACOBI");
	free(b);
	free(z);
}


//! Perform the Cholesky decomposition
//! \param a the matrix we are operating on
//! \param n size of (NxN) matrix
//! \param l the lower triangular L  such that L*L'=A
void Ellipsenfit::choldc(double** a, int n, double** l) {
	int i,j,k;
	double sum;
	double* p;
	p=(double*)malloc((n+1)*sizeof(double));

	for (i=1; i<=n; i++)  {
		for (j=i; j<=n; j++)  {
			for (sum=a[i][j],k=i-1;k>=1;k--) sum -= a[i][k]*a[j][k];
			if (i == j) {
				if (sum<=0.0)
					// printf("\nA is not poitive definite!");
				{}
				else
					p[i]=sqrt(sum); }
			else
			{
				a[j][i]=sum/p[i];
			}
		}
	}
	for (i=1; i<=n; i++)
		for (j=i; j<=n; j++)
			if (i==j)
				l[i][i] = p[i];
			else
			{
				l[j][i]=a[j][i];
				l[i][j]=0.0;
			}
	free(p);
}

//! computes inverse of given matric
//! \param TB the matrix we are operating on
//! \param InvB result
//! \param N size of (NxN) matrices
//! \return 0 if successful
int Ellipsenfit::inverse(double** TB, double** InvB, int N) {

	int k,i,j,q;
	double mult;
	double D,temp;
	double maxpivot;
	int npivot;
	//	double B[N+1][N+2];
	double** B;
	B=(double**)malloc((N+1)*sizeof(double*));
	for (int i = 0; i<(N+1);i++){
		B[i]=(double*)malloc((N+2)*sizeof(double));
	}

	//	double A[N+1][2*N+2];
	double** A;
	A=(double**)malloc((N+1)*sizeof(double*));
	for (int i = 0; i<(N+1);i++){
		A[i]=(double*)malloc((2*N+2)*sizeof(double));
	}
	//	double C[N+1][N+1];
	double eps = 10e-10;


	for(k=1;k<=N;k++)
		for(j=1;j<=N;j++)
			B[k][j]=TB[k][j];

	for (k=1;k<=N;k++)
	{
		for (j=1;j<=N+1;j++)
			A[k][j]=B[k][j];
		for (j=N+2;j<=2*N+1;j++)
			A[k][j]=(float)0;
		A[k][k-1+N+2]=(float)1;
	}
	for (k=1;k<=N;k++)
	{
		maxpivot=abs((double)A[k][k]);
		npivot=k;
		for (i=k;i<=N;i++)
			if (maxpivot<abs((double)A[i][k]))
			{
				maxpivot=abs((double)A[i][k]);
				npivot=i;
			}
		if (maxpivot>=eps)
		{      if (npivot!=k)
			for (j=k;j<=2*N+1;j++)
			{
				temp=A[npivot][j];
				A[npivot][j]=A[k][j];
				A[k][j]=temp;
			} ;
			D=A[k][k];
			for (j=2*N+1;j>=k;j--)
				A[k][j]=A[k][j]/D;
			for (i=1;i<=N;i++)
			{
				if (i!=k)
				{
					mult=A[i][k];
					for (j=2*N+1;j>=k;j--)
						A[i][j]=A[i][j]-mult*A[k][j] ;
				}
			}
		}
		else
		{  // printf("\n The matrix may be singular !!") ;
			for (int i=0;i<(N+1);i++) {
				free(A[i]);
				free(B[i]);
			}
			free(A);
			free(B);
			return(-1);
		};
	}
	/**   Copia il risultato nella matrice InvB  ***/
	for (k=1;k<=N;k++)
		for (q=1;q+N+1<=2*N+1;q++)
			InvB[k][q]=A[k][q+N+1];
	for (int i=0;i<(N+1);i++) {
		free(A[i]);
		free(B[i]);
	}
	free(A);
	free(B);
	return(0);
}            /*  End of INVERSE   */


//! multipies matrix a with matrix b
//! \param _A first matrix
//! \param _B second matrix
//! \param _res result matrix
//! \param _righA row count of first matrix
//! \param _colA column count of first matrix
//! \param _righB row count of second matrix
//! \param _colB column count of secound matrix
void Ellipsenfit::AperB(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB) {
	int p,q,l;
	p= _righB;
	for (p=1;p<=_righA;p++)
		for (q=1;q<=_colB;q++)
		{ _res[p][q]=0.0;
		for (l=1;l<=_colA;l++)
			_res[p][q]=_res[p][q]+_A[p][l]*_B[l][q];
		}
}

//! transposes first matrix and then multipies first matrix with second matrix 
//! \param _A first matrix
//! \param _B second matrix
//! \param _res result matrix
//! \param _righA row count of first matrix
//! \param _colA column count of first matrix
//! \param _righB row count of second matrix
//! \param _colB column count of secound matrix
void Ellipsenfit::A_TperB(double** _A, double**  _B, double** _res, int _righA, int _colA, int _righB, int _colB) {
	int p,q,l;
	p= _righB;
	for (p=1;p<=_colA;p++)
		for (q=1;q<=_colB;q++)
		{ _res[p][q]=0.0;
		for (l=1;l<=_righA;l++)
			_res[p][q]=_res[p][q]+_A[l][p]*_B[l][q];
		}
}

//! transposes second matrix and then multipies first matrix with second matrix
//! \param _A first matrix
//! \param _B second matrix
//! \param _res result matrix
//! \param _righA row count of first matrix
//! \param _colA column count of first matrix
//! \param _righB row count of second matrix
//! \param _colB column count of secound matrix
void Ellipsenfit::AperB_T(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB) {
	int p,q,l;
	p= _righB;
	for (p=1;p<=_colA;p++)
		for (q=1;q<=_colB;q++)
		{ _res[p][q]=0.0;
		for (l=1;l<=_righA;l++)
			_res[p][q]=_res[p][q]+_A[p][l]*_B[q][l];
		}
}

//! Direct Least Square Fitting of Ellipses by A. Fitzgibbon, M. Pilu , R.Fisher
//! from Java-Code --> http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/PILU1/demo.html
//! \param at least six points to fit an ellipse to
//! \return error, centerX, centerY, radius1, radius2, theta
vector<double> Ellipsenfit::findEllipseParams(int mode, vector<Vector3D> Schnittpunkte) {
	if (mode == CONIC) return toconic(Schnittpunkte);

	size_t np = Schnittpunkte.size();           // number of points
	double tx,ty;
	int nrot=0;

	double** D;
	double** S;
	double** L;
	double** invL;
	double** Const;
	double** temp;
	double** C;
	double** V;
	double** sol;
	double* d;
	vector<double> pvec;

	pvec.resize(7,0.0);

	d= (double *) malloc(7* sizeof(double));
	Const= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		Const[i]= (double *) malloc(7* sizeof(double));
	}
	S= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		S[i]= (double *) malloc(7* sizeof(double));
	}
	temp= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		temp[i]= (double *) malloc(7* sizeof(double));
	}
	L= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		L[i]= (double *) malloc(7* sizeof(double));
	}
	C= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		C[i]= (double *) malloc(7* sizeof(double));
	}
	invL= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		invL[i]= (double *) malloc(7* sizeof(double));
	}
	V= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		V[i]= (double *) malloc(7* sizeof(double));
	}
	sol= (double **) malloc(7* sizeof(double *));
	for (int i= 0; i< 7; i++){
		sol[i]= (double *) malloc(7* sizeof(double));
	}
	D= (double **) malloc((np+1)* sizeof(double *));
	for (size_t i= 0; i< np+1; i++){
		D[i]= (double *) malloc(7* sizeof(double));
	}
	if (mode == FPF) {
		Const[1][3]=-2;
		Const[2][2]=1;
		Const[3][1]=-2;
	}

	if (mode == BOOKSTEIN) {
		Const[1][1]=2;
		Const[2][2]=1;
		Const[3][3]=2;
	}

	// Now first fill design matrix
	for (size_t i=0; i < np; i++) {
		tx = (double) Schnittpunkte[i].getZ();
		ty = (double) Schnittpunkte[i].getX();
		D[(int)i+1][1] = tx*tx;
		D[(int)i+1][2] = tx*ty;
		D[(int)i+1][3] = ty*ty;
		D[(int)i+1][4] = tx;
		D[(int)i+1][5] = ty;
		D[(int)i+1][6] = 1.0;
	}

	// Now compute scatter matrix  S
	A_TperB(D,D,S,(int)np,6,(int)np,6);

	choldc(S,6,L);

	inverse(L,invL,6);

	AperB_T(Const,invL,temp,6,6,6,6);
	AperB(invL,temp,C,6,6,6,6);

	jacobi(C,6,d,V,nrot);

	A_TperB(invL,V,sol,6,6,6,6);

	// Now normalize them
	for (int j=1;j<=6;j++)  /* Scan columns */
	{
		double mod = 0.0;
		for (int i=1;i<=6;i++)
			mod += sol[i][j]*sol[i][j];
		for (int i=1;i<=6;i++)
			sol[i][j] /=  sqrt(mod);
	}

	double zero=10e-20;
	double minev=10e+20;
	int  solind=0;

	if (mode == FPF) {
		for (int i=1; i<=6; i++)
			if (d[i]<0 && abs(d[i])>zero)
				solind = i;
	}

	if (mode == BOOKSTEIN) {
		for (int i=1; i<=6; i++)
			if (d[i]<minev && abs(d[i])>zero)
				solind = i;
	}

	// Now fetch the right solution
	for (int j=1;j<=6;j++)
		pvec[j] = sol[j][solind];

	// ...and solve polynom to standard form
	vector<double> res = solveellipse(pvec);

	for (int i= 0; i< 7; i++){
		free(S[i]);
		free(Const[i]);
		free(temp[i]);
		free(L[i]);
		free(C[i]);
		free(invL[i]);
		free(V[i]);
		free(sol[i]);
	}
	free(S);
	free(Const);
	free(temp);
	free(L);
	free(C);
	free(invL);
	free(V);
	free(sol);
	//	free(pvec);
	free(d);

	for (size_t i= 0; i< np+1; i++){
		free(D[i]);
	}
	free(D);
	return res;
}

//! ellipse polygon to standard form from MatLab-Code
//! -->  http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ELLIPFIT/solveellipse.m
//! Given an ellipse in polynomial form, finds the standard form: ((x-cx)/r1)^2 + ((y-cy)/r2)^2 = 1
//! error < 0 indicates a failed fit containing "NotANumber"-values
//! \param ellipsePolynom: a(1)x^2 + a(2)xy + a(3)y^2 + a(4)x + a(5)y + a(6) = 0
//! \return error, centerX, centerY, radius1, radius2, theta
vector<double> Ellipsenfit::solveellipse(vector<double> a) {
	vector<double> res;
	res.resize(6,0.0);
	// get ellipse orientation
	//theta = atan2(a(2),a(1)-a(3))/2;
	res[5] = atan2(a[2],a[1]-a[3])/2;

	// get scaled major/minor axes
	//        ct = cos(theta);
	double ct = cos(res[5]);
	//        st = sin(theta);
	double st = sin(res[5]);
	//        ap = a(1)*ct*ct + a(2)*ct*st + a(3)*st*st;
	double ap = a[1]*ct*ct + a[2]*ct*st + a[3]*st*st;
	//        cp = a(1)*st*st - a(2)*ct*st + a(3)*ct*ct;
	double cp = a[1]*st*st - a[2]*ct*st + a[3]*ct*ct;

	double** T;
	T = (double**) malloc(3*sizeof(double*));
	T[1] = (double*) malloc(3*sizeof(double));
	T[2] = (double*) malloc(3*sizeof(double));

	double** invT;
	invT = (double**) malloc(3*sizeof(double*));
	invT[1] = (double*) malloc(3*sizeof(double));
	invT[2] = (double*) malloc(3*sizeof(double));

	//        T = [[a(1) a(2)/2]' [a(2)/2 a(3)]'];
	T[1][1] = 2*a[1];
	T[2][1] = 2*a[2]/2;
	T[1][2] = 2*a[2]/2;
	T[2][2] = 2*a[3];

	double* t;
	t = (double*) malloc(3*sizeof(double));

	//        t = -inv(2*T)*[a(4) a(5)]';
	inverse(T,invT,2);
	invT[1][1] *= -1;
	invT[1][2] *= -1;
	invT[2][1] *= -1;
	invT[2][2] *= -1;

	t[1] = invT[1][1]*a[4]+invT[1][2]*a[5];
	t[2] = invT[2][1]*a[4]+invT[2][2]*a[5];

	//        cx = t(1);
	//        cy = t(2);
	res[1] = t[1];
	res[2] = t[2];

	// get scale factor
	//        val = t'*T*t;
	//        scale = 1 / (val- a(6));

	T[1][1] *= 0.5;
	T[1][2] *= 0.5;
	T[2][1] *= 0.5;
	T[2][2] *= 0.5;

	double val = (T[1][1]*t[1]+T[2][1]*t[2])*t[1]+(T[1][2]*t[1]+T[2][2]*t[2])*t[2];
	double scale = 1 / (val- a[6]);

	// get major/minor axis radii
	//        r1 = 1/sqrt(scale*ap);
	//        r2 = 1/sqrt(scale*cp);
	//        v = [r1 r2 cx cy theta]';

	res[3] = 1/sqrt(scale*ap);
	res[4] = 1/sqrt(scale*cp);
	res[5]*=180/M_PI;

	// catching errors that happen when a fit fails
	if (std::isnan(res[3])) res[0]-=1;
	if (std::isnan(res[4])) res[0]-=1;

	free(T[1]);
	free(T[2]);
	free(T);
	free(t);
	free(invT[1]);
	free(invT[2]);
	free(invT);
	return res;
}
