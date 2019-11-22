#include "geodentry.h"

//! Constructor
GeodEntry::GeodEntry( double rDist, double rAngle, Primitive* rFromSeed )
	: mGeodDist( rDist ), mGeodAngle( rAngle ), mFromSeed( rFromSeed ) {
	//cout << "[GeodEntry::" << __FUNCTION__ << "] Angle: " << rAngle << endl;
	if( rAngle < -M_PI ) {
		mGeodAngle = -( rAngle + M_PI );
	}
	if( rAngle > +M_PI ) {
		mGeodAngle = -( rAngle - M_PI );
	}
}

//! Updates the entry only, when rDist is smaller.
//! @returns true, when an update was made -- false otherwise.
bool GeodEntry::setGeodDistSmaller( double rDist, double rAngle, Primitive* rFromSeed ) {
	if( rDist > mGeodDist ) {
		return false;
	}
	mGeodDist = rDist;
//	if( rDist == mGeodDist ) {
//		mGeodAngle = ( mGeodAngle + rAngle ) / 2.0;
//	} else {
//		mGeodAngle = rAngle;
//	}
	mGeodAngle = rAngle;
	if( rAngle < -M_PI ) {
		mGeodAngle = -( rAngle + M_PI );
	}
	if( rAngle > +M_PI ) {
		mGeodAngle = -( rAngle - M_PI );
	}
	mFromSeed = rFromSeed;
	return true;
}

//! Retrieve the geodisc distance.
//! @returns false in case of an error.
bool GeodEntry::getGeodDist( double* rDist ) {
	(*rDist) = mGeodDist;
	return true;
}

//! Retrieve the geodisc angle.
//! @returns false in case of an error.
bool GeodEntry::getGeodAngle( double* rAngle ) {
	(*rAngle) = mGeodAngle;
	return true;
}

//! Retrieve the pointer to the seed Primitive.
//! @returns false in case of an error.
bool GeodEntry::getFromSeed( Primitive** rFromSeed ) {
	(*rFromSeed) = mFromSeed;
	return true;
}

//! Returns the label id of the seed Primitive -- see Primitive::getLabelID
//! @returns false in case of an error.
bool GeodEntry::getLabel( uint64_t& rLabelId ) const {
	return mFromSeed->getLabel( rLabelId );
}
