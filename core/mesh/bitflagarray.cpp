#include <GigaMesh/mesh/bitflagarray.h>

#define BITFLAGARRAY_INITDEFAULTS \
	mBitFlags( 0x00000000 )

//! Constructor
BitFlagArray::BitFlagArray()
	: BITFLAGARRAY_INITDEFAULTS {
}


// Single flags ------------------------------------------------------------------------------------------------------------------------------------------------

//! Checks if one (or more) flag(s) are set - see BitFlagArray::mBitFlags
bool BitFlagArray::getFlag( uint64_t rFlagNr ) const {
	return ( mBitFlags & rFlagNr );
}

//! Sets a given flag to true - see BitFlagArray::mBitFlags
bool BitFlagArray::setFlag( uint64_t rFlagNr ) {
	mBitFlags |= rFlagNr;
	return true;
}

//! Sets a given flag to false - see BitFlagArray::mBitFlags
bool BitFlagArray::clearFlag( uint64_t rFlagNr ) {
	mBitFlags &= ~rFlagNr;
	return true;
}

// All flags ---------------------------------------------------------------------------------------------------------------------------------------------------

//! Set bit flag array to rFlags.
bool BitFlagArray::getFlagAll( uint64_t* rFlags ) const {
	(*rFlags) = mBitFlags;
	return true;
}

//! Set bit flag array to rFlags.
bool BitFlagArray::setFlagAll( uint64_t rFlags ) {
	mBitFlags = rFlags;
	return true;
}
