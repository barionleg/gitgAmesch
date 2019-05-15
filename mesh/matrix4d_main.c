#include <stdio.h>  
#include <stdlib.h> // calloc

#include "matrix4d.h"

#include "gmcommon.h" // for windows

using namespace std;

int main(void) {  
	//! Main routine for testing Matrix4D
	//==========================================================================

	Matrix4D identityMatrix( _MATRIX4D_INIT_IDENTITY_ );

	vector<double> rotX90 { M_PI/2.0 };
	Matrix4D transformMatrix1( Matrix4D::INIT_ROTATE_ABOUT_X, &rotX90 );
	// old - deprecated: Matrix4D transformMatrix1( _MATRIX4D_INIT_ROTATION_X_, M_PI/2.0 );
	transformMatrix1.dumpInfo();

	identityMatrix *= transformMatrix1;

	cout << "--------------------------------------------------------------------------------" << endl;

	Matrix4D transformMatrix2( _MATRIX4D_INIT_ROTATION_Y_, M_PI/4.0 );
	transformMatrix2.dumpInfo();


	cout << "--------------------------------------------------------------------------------" << endl;

	transformMatrix1 *= transformMatrix2;
	transformMatrix1.dumpInfo();

	cout << "--------------------------------------------------------------------------------" << endl;

	identityMatrix *= transformMatrix2;
	identityMatrix.dumpInfo();

	cout << "--------------------------------------------------------------------------------" << endl;

	// someMat = [ 7    5    9    0    2    6    0    5    9    6    7   10    4    9    5    9 ];
	// reshape( someMat, 4, 4 );
	// inv( reshape( someMat, 4, 4 ) )
	double      someMatrix[16]    = { 7, 5, 9, 0, 2, 6, 0, 5, 9, 6, 7, 10, 4, 9, 5, 9 };
	const float someMatrixFLT[16] = { 7, 5, 9, 0, 2, 6, 0, 5, 9, 6, 7, 10, 4, 9, 5, 9 };
	float       someMatrixFLTInverted[16];
	Matrix4D matrixToInvert( someMatrix );
	matrixToInvert.dumpInfo();
	cout << "... old: ......................................................................." << endl;
	// working with older code:
	invert( someMatrixFLT, someMatrixFLTInverted );
	Matrix4D matrixFLTInverted( someMatrixFLTInverted );
	matrixFLTInverted.dumpInfo();
	cout << "... new: ........................................................................" << endl;
	// new version:
	matrixToInvert.invert();
	matrixToInvert.dumpInfo();

	cout << "--------------------------------------------------------------------------------" << endl;
} 
