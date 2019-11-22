#include <stdio.h>  
#include <stdlib.h> // calloc, uint, ...
#include <iostream> // cout, endl
#include "limits.h" // UCHAR_MAX, ...
#include <string>
#include <sys/stat.h> // statistics for files
#include <fstream>
//#include <iostream>
#include <algorithm>

using namespace std;

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

int main( int argc, char *argv[] ) {
	//! Converter
	//==========================================================================

	string fileNameIn;
	string fileNameOut;
	string nodePrefix;

	int c;
	while( ( c = getopt( argc, argv, "f:p:" ) ) != -1 ) {
		switch( c ) {
			//! Option f: filename for input data (required)
			case 'f':
				fileNameIn = optarg;
				break;
			case 'p':
				nodePrefix = optarg;
				break;
			// //! Option o: filename for input data (required)
			//case 'o':
			//	fileNameOut = optarg;
			//	break;
			default:
				cerr << "Error: Unknown option '" << c << "'!" << endl;
				exit( EXIT_FAILURE );
		}
	}

	// Check files using file statistics
	struct stat stFileInfo;
	// Check: Input file exists?
	if( stat( fileNameIn.c_str(), &stFileInfo ) != 0 ) {
		cerr << "Error: File '" << fileNameIn << "' not found!" << endl;
		exit( EXIT_FAILURE );
	}

	ifstream filestrIN( fileNameIn.c_str() );
	//ofstream filestrOUT( fileNameOut.c_str() );
	char fileLine[1024];

	string currUser;
	string currTime;

	while( filestrIN.good() ) {
		filestrIN.getline( fileLine, 1024 );
		//cout << fileLine << endl;
		string currLine( fileLine );
		if( currLine.length() == 0 ) {
			continue;
		}
		if( currLine.compare( 0, 5, "user:" ) == 0 ) {
			currUser = currLine.substr( 5 );
			trim( currUser );
			continue;
		}
		if( currLine.compare( 0, 1, ":" ) == 0 ) {
			// e.g: ":000000 100644 0000000... 44c46a1... A	.gitignore"
			// line - tab delimits the filename
			int splitPos = currLine.find_last_of( '\t' );
			string currType = currLine.substr( splitPos-1, 1 );
			string currFile = currLine.substr( splitPos );
			trim( currFile );
			cout << currTime << "|" << currUser << "|" << currType << "|" << nodePrefix << currFile << endl;
			continue;
		}
		// timestamp;
		currTime = currLine;
	}

	filestrIN.close();
	//filestrOUT.close();

	exit( EXIT_SUCCESS );
}
