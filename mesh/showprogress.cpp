#include "showprogress.h"

#include<iostream>
#include<cmath>
#include <chrono>

using namespace std;

ShowProgress::ShowProgress() {
	// Nothing to do here.
}

// --- Progressbar ---------------------------------------------------------------------------------------------------------------------------------------------

//! Stub for higher level progress bars, e.g. to open a window.
void ShowProgress::showProgressStart(
                const string& rMsg
) {
	mProgressStarted = clock();
	mLastTimeStamp = static_cast<double>(mProgressStarted);
	cout << "[Mesh] " << rMsg << " --- Begin." << endl;
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
                const string& rMsg
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
	string timeRemainingUnit = "sec";
	if( timeRemaining > ( 24.0 * 3600.0 ) ) {
		timeRemaining = round( 10.0 * timeRemaining / ( 24.0 * 3600.0 ) ) / 10.0;
		timeRemainingUnit = "DAYS";
	} else if( timeRemaining > 3600.0 ) {
		timeRemaining = round( 10.0 * timeRemaining / 3600.0 ) / 10.0;
		timeRemainingUnit = "hours";
	}

	// Show progress on the console:
	cout << "[Mesh] " << rMsg << " ";
	cout << "| " << round( rVal*1000.0 )/10.0 << "% ";
	cout << "| " << round( timeElapsed*10.0 )/10.0 << " sec ";
	cout << "elapsed - ";
	cout << "remaining: " << timeRemaining << " " << timeRemainingUnit;
	cout << endl;

	return( true );
}

//! Stub for higher level progress bars, e.g. to close a window.
void ShowProgress::showProgressStop(
                const string& rMsg
) {
	cout << "[Mesh] " << rMsg << " --- Done." << endl;
}
