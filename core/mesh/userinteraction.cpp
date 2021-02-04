//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

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
	std::cout << rHead << std::endl;
	std::cout << rMsg  << std::endl;
}

//! Stub for higher level warnings. e.g. Messagebox with Qt.
void UserInteraction::showWarning(
                const string& rHead,
                const string& rMsg
) {
	std::cout << rHead << std::endl;
	std::cout << rMsg  << std::endl;
}

//! Stub for higher level methods to enter a string.
bool UserInteraction::showEnterText( std::string& rSomeStrg, const char* rTitle ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter a single unsigned integer value.
bool UserInteraction::showEnterText( uint64_t& rULongInt, const char* rTitle ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter a single double precision floating point value.
bool UserInteraction::showEnterText( double& rDoubleVal, const char* rTitle ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter integer values e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( std::set<int64_t>& rIntegers, const char* rTitle ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter integer values e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( std::vector<long>& rIntegers, const char* rTitle ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter double precision values e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( std::vector<double>& rDoubles, const char* rTitle ) {
	cerr << "[UserInteraction::" << __FUNCTION__ << "] ERROR: not implemented for commandlines use!" << endl;
	return false;
}

//! Stub for higher level methods to enter 16 floating point values for a 4x$ Matrix e.g. using QGMDialogEnterText.
bool UserInteraction::showEnterText( Matrix4D* rMatrix4x4, bool selectedVerticesOnly ) {
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
