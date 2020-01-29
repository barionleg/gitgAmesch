#ifndef ELLIPSEDISC_H
#define ELLIPSEDISC_H

#include "primitive.h"

class EllipseDisc : public Primitive { // Ellipse is a win32 function :(

public:
	EllipseDisc() = default;

	enum eComputeMode {
		BOOKSTEIN,
		FPF,
		CONIC
	};

	bool findEllipseParams( eComputeMode rMode, std::vector<std::pair<double,double> > rPoints );
	bool toconic( std::vector<std::pair<double,double> > rFivePoints );

	void dumpInfo();

private:
	void AperB_T(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	void A_TperB(double** _A, double**  _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	void AperB(double** _A, double** _B, double** _res, int _righA, int _colA, int _righB, int _colB);
	int inverse(double** TB, double** InvB, int N);
	void choldc(double** a, int n, double** l);
	void jacobi(double** a, int n, double d[] , double** v, int nrot);
	bool solveellipse( std::vector<double> a );
	void ROTATE(double** a, int i, int j, int k, int l, double tau, double s);

	// Ellipse definition (in 2D)
	double mError;
	double mCenterX;
	double mCenterY; 
	double mRadius1;
	double mRadius2;
	double mTheta;
};

#endif // ELLIPSEDISC_H
