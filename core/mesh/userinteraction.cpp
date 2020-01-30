#include <GigaMesh/mesh/userinteraction.h>

#include<iostream>

using namespace std;

//! Constructor.
UserInteraction::UserInteraction() {
	// Nothing to do.
}

//! Stub for higher level warnings. e.g. Messagebox with Qt.
void UserInteraction::showInformation(
                const std::string& rHead,
                const std::string& rMsg,
                [[maybe_unused]] const std::string& rToClipboard
) {
	std::cout << rHead << " " << rMsg << std::endl;
}

//! Stub for higher level warnings. e.g. Messagebox with Qt.
void UserInteraction::showWarning(
                const string& rHead,
                const string& rMsg
) {
	cout << rHead << " " << rMsg << endl;
}

//! Stub for higher level methods to enter a string.
bool UserInteraction::showEnterText( std::string& rSomeStrg , const char* rTitle  ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter a single unsigned integer value.
bool UserInteraction::showEnterText( uint64_t& rULongInt , const char* rTitle  ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter a single double precision floating point value.
bool UserInteraction::showEnterText( double& rDoubleVal , const char* rTitle   ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter integer values e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( set<long>& rIntegers , const char* rTitle  ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter integer values e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( vector<long>& rIntegers , const char*  rTitle  ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter double precision values e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( vector<double>& rDoubles ,  const char* rTitle  ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter 16 floating point values for a 4x$ Matrix e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( Matrix4D* rMatrix4x4  ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to choose a value using a slider e.g. using QGMDialogSlider.
bool UserInteraction::showSlider(
                double* rValueToChange ,
                double  rMin ,
                double  rMax ,
                const char* rTitle
                //void* rSlot
) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for: Let the user choose a boolean value.
//! @returns true, when a valid value was choosen and confirmed by the user. False otherwise i.e. user cancel or error.
bool UserInteraction::showQuestion(
                bool* rUserChoice     ,
                const string& rHead   ,
                const string& rMsg
) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}
