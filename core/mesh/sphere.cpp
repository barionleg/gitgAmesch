#include <GigaMesh/mesh/sphere.h>

using namespace std;

Sphere::Sphere( Vector3D vertPos, float setRadius, unsigned char setR, unsigned char setG, unsigned char setB )
	: Vertex( vertPos ) {
	//! Constructor - requires only coordiantes and radii. Memory will be allocated. Optional a color can be set.
	//!
	//! This constructor is more convinient as it does not require an array to be handled outside this object.
	//!
	//! But: e.g. if this object has to be transfomed, this has to be done with its own method. 
	//!
	//! Or in other words: using this Constructor prevents CUDA/OpenCL integration and should therefore only be used 
	//! for testing or simple visualizations.
	radius = setRadius;
	if( !setRGB( setR, setG, setB ) ) {
		cerr << "[Sphere::" << __FUNCTION__ << "] ERROR: Could not set RGB!" << endl;
	}
}

Sphere::~Sphere() {
	//! Destructor. sets properties to not a number or NULL.
	radius = _NOT_A_NUMBER_;
}

float Sphere::getRadius() {
	//! Returns the radius of the Sphere.
	return radius;
}

int Sphere::getType() {
	//! Return the type (=class) of this object as an id.
	return Primitive::IS_SPHERE;
}
