#ifndef GEODENTRY_H
#define GEODENTRY_H

#include "primitive.h"

class Primitive;

//!
//! \brief Struct for storing a geodesic distance and the Primitive it is closest to. (Layer -0.5)
//!
//! ...
//!
//! Layer -0.5
//!

class GeodEntry {
	public:
		GeodEntry( double rDist, double rAngle, Primitive* rFromSeed );
		~GeodEntry() = default;

		bool setGeodDistSmaller( double rDist, double rAngle, Primitive* rFromSeed );

		bool getGeodDist( double* rDist );
		bool getGeodAngle( double* rAngle );
		bool getFromSeed( Primitive** rFromSeed );

		bool getLabel( uint64_t& rLabelId ) const;

	private:
		double     mGeodDist;  //!< The geodesic distance.
		double     mGeodAngle; //!< Experimental: angle related to geodesics
		Primitive* mFromSeed;  //!< Reference to the seed Primitive.
};

#endif
