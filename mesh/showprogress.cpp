#include "showprogress.h"

#include <iostream>
#include <cmath>
#include <chrono>

ShowProgress::ShowProgress(
                const std::string& rPrefix
) {
	mPrefix = rPrefix;
	// Append space, when a string is given.
	if( mPrefix.size() > 0 ) {
		mPrefix += " ";
	}
}

// --- Progressbar ---------------------------------------------------------------------------------------------------------------------------------------------

//! Stub for higher level progress bars, e.g. to open a window.
void ShowProgress::showProgressStart(
                const std::string& rMsg
) {
	mProgressStarted = clock();
	mLastTimeStamp = static_cast<double>(mProgressStarted);
	std::cout << mPrefix << rMsg << " --- Begin." << std::endl;
}

//! Show progress bar.
//! Suppress updates until a time threshold is reached.
//! If rVal >= 1.0 than a new line is shown.
//!
//! @param rVal .... range [0.0,1.0]
//! @param rMsg some Msg-Prefix
//! @returns false, when supressing output and/or function calls (for higher level derivates).
bool ShowProgress::showProgress(
                double rVal,
                const std::string& rMsg
) {
	// Fetch timestamp from last update.
	double lastSignal = mLastTimeStamp;
	if( ( ( clock() - lastSignal ) / CLOCKS_PER_SEC ) < 2.0 ) { // Wait between firing signals.
		// Do nothing to prevent a DOS by sending to many signals.
		return( false );
	}
	// Set timestamp
	mLastTimeStamp = static_cast<double>(clock());

	//! \todo there are certainly more elegant ways to format time into useful/readable units.
	// Compute progress on the console:
	double timeElapsed = ( mLastTimeStamp - mProgressStarted ) / CLOCKS_PER_SEC;
	double timeRemaining = round( 10.0 * ( timeElapsed / rVal ) * ( 1.0 - rVal ) ) / 10.0;
	std::string timeRemainingUnit = "sec";
	if( timeRemaining > ( 24.0 * 3600.0 ) ) {
		timeRemaining = round( 10.0 * timeRemaining / ( 24.0 * 3600.0 ) ) / 10.0;
		timeRemainingUnit = "DAYS";
	} else if( timeRemaining > 3600.0 ) {
		timeRemaining = round( 10.0 * timeRemaining / 3600.0 ) / 10.0;
		timeRemainingUnit = "hours";
	}

	// Show progress on the console:
	std::cout << mPrefix << rMsg << " ";
	std::cout << "| " << round( rVal*1000.0 )/10.0 << "% ";
	std::cout << "| " << round( timeElapsed*10.0 )/10.0 << " sec ";
	std::cout << "elapsed - ";
	std::cout << "remaining: " << timeRemaining << " " << timeRemainingUnit;
	std::cout << std::endl;

	return( true );
}

//! Stub for higher level progress bars, e.g. to close a window.
void ShowProgress::showProgressStop(
                const std::string& rMsg
) {
	double timeElapsed = ( static_cast<double>(clock()) - mProgressStarted ) / CLOCKS_PER_SEC;
	std::cout << mPrefix << rMsg << " --- Done."
	             " Processing took " << timeElapsed << " seconds." << std::endl;
}
