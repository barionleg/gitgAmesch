#ifndef GMCOMMON_H
#define GMCOMMON_H

// generic C++
#include <limits>    // numeric_limits
#include <limits.h>  // CHAR_MAX, etc.
#include <iomanip>   // for setiosflags( ios_base::fixed ) - see: http://www.cplusplus.com/reference/iostream/ios_base/fmtflags/
#include <string.h>  // memcpy, strncmp
#include <float.h>   // FLT_MAX, FLT_MIN, FLT_EPS, etc.

// generic defines
#define _INFINITE_DBL_      std::numeric_limits<double>::infinity()
#define _NOT_A_NUMBER_INT_  std::numeric_limits<int>::quiet_NaN()
#define _NOT_A_NUMBER_UINT_ std::numeric_limits<unsigned int>::quiet_NaN()
#define _NOT_A_NUMBER_LONG_    std::numeric_limits<long>::quiet_NaN()
#define _NOT_A_NUMBER_ULONG_   std::numeric_limits<uint64_t>::quiet_NaN()
#define _NOT_A_NUMBER_DBL_  std::numeric_limits<double>::quiet_NaN()
#define _NOT_A_NUMBER_      std::numeric_limits<float>::quiet_NaN()

// because "%" works differently for negative numbers in C/C++
#define MODULO_INT( a, b ) \
	(int)( a-(b*floor((float)a/(float)b)) ) \

// Windows: Because mingw has no M_PI defined, we have to define it:
#ifndef M_PI
//#warning "Defining M_PI for windows compatibility"
#define M_PI           3.14159265358979323846
#endif

// Windows: uses another random call:
#ifdef WIN32
//#warning "Redefining random(x) to rand(x) for windows compatibility"
#define random(x) rand(x)
#ifdef __FUNCDNAME__	//this only works with msvc. mingw seems to have problems with it.
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#endif
#endif

// Struct to hold vertex properties used to read/write
struct sVertexProperties {
	double mCoordX  = _NOT_A_NUMBER_DBL_; // vertex coordinate
	double mCoordY  = _NOT_A_NUMBER_DBL_;
	double mCoordZ  = _NOT_A_NUMBER_DBL_;
	double mNormalX = _NOT_A_NUMBER_DBL_; // per vertex normal
	double mNormalY = _NOT_A_NUMBER_DBL_;
	double mNormalZ = _NOT_A_NUMBER_DBL_;
	double mFuncVal = _NOT_A_NUMBER_DBL_; // Scalar value typically stored within the PLY's quality field.
	unsigned char mColorRed = 0; // Red
	unsigned char mColorGrn = 0; // Green
	unsigned char mColorBle = 0; // Blue
	unsigned char mColorAlp = 0; // Alpha
	unsigned long mLabelId = 0; // Connected component id. Zero means background.
	unsigned long mFlags = 0; // Flags
};

struct sFaceProperties {
	unsigned long mVertIdxA;
	unsigned long mVertIdxB;
	unsigned long mVertIdxC;
};

#endif
