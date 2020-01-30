#ifndef BITFLAGARRAY_H
#define BITFLAGARRAY_H

#include <cstdint>

class BitFlagArray {

public:
	// Constructor
	BitFlagArray();
	virtual ~BitFlagArray() = default;

	// Pre-define flags
	enum eBitFlags {
		FLAG_00 = 1U<<0,
		FLAG_01 = 1U<<1,
		FLAG_02 = 1U<<2,
		FLAG_03 = 1U<<3,
		FLAG_04 = 1U<<4,
		FLAG_05 = 1U<<5,
		FLAG_06 = 1U<<6,
		FLAG_07 = 1U<<7,
		FLAG_08 = 1U<<8,
		FLAG_09 = 1U<<9,
		FLAG_10 = 1U<<10,
		FLAG_11 = 1U<<11,
		FLAG_12 = 1U<<12
	};

	// Single flags
	bool     getFlag( uint64_t rFlagNr ) const;
	bool     setFlag( uint64_t rFlagNr );
	bool     clearFlag( uint64_t rFlagNr );
	// All flags
	bool     getFlagAll( uint64_t* rFlags ) const;
	bool     setFlagAll( uint64_t rFlags );

private:
	// One BitArray for 64 flags:
	uint64_t mBitFlags;     //! perfect for a 64-bit system - we can handle 64 flags

};

#endif // BITFLAGARRAY_H
