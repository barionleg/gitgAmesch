#ifndef ELLIPSENFIT_H_
#define ELLIPSENFIT_H_

#include <cmath>
#include <cstdlib>

#include "vector3d.h"
#include "vertex.h"
#include "matrix4d.h"
#include "mesh.h"


class Ellipsenfit {

public:
	Ellipsenfit();
	Ellipsenfit(set<Face*> faceList, Vector3D HNF_normale, Vector3D HNF_position); //, Vector3D direction);
	~Ellipsenfit();

	Vector3D averageCenter(vector<Vector3D> points);
	vector<Vector3D> deleteOutlier(vector<Vector3D> points);
	vector<vector<double> > deleteOutlier(vector<vector<double> > points);
	bool edgeCheck(Vector3D A, Vector3D B);
	Vector3D findEdgeCut(Vector3D edgeVertA, Vector3D edgeVertB);
	vector<double> findEllipseParams(int mode, vector<Vector3D> Schnittpunkte);
	void findCuttingPoints(set<Face*> faceList);
	vector<double> toconic(vector<Vector3D> subset);
	Matrix4D operationFull;
	Matrix4D operationInverse;
	Vector3D HNF_normale;
	Vector3D HNF_position;
	Vector3D rotationsAchse;
	float angleToY;
	vector<Vector3D> reduceNumberOfPoints(size_t pointsCount);
	vector<Vector3D> reduceNumberOfPointsRandomly(size_t pointsCount);
	vector<double> getEllipseParameter();


private:
	void AperB_T(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	void A_TperB(double** _A, double**  _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	void AperB(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	int inverse(double** TB, double** InvB, int N);
	void choldc(double** a, int n, double** l);
	void jacobi(double** a, int n, double d[] , double** v, int nrot);
	void ROTATE(double** a, int i, int j, int k, int l, double tau, double s);
	vector<double> solveellipse(vector<double> a);
	bool isPointInVector(vector<Vector3D> theVector, Vector3D thePoint);
	double testCircularity(double r1, double r2);
	Matrix4D operationTranslate;
	Matrix4D operationRotate;
	vector<Vector3D> Schnittpunkte;
};

#endif /* ELLIPSENFIT_H_ */
