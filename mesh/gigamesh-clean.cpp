#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

#ifdef WIN32	//windows version for hostname and login
//#include <winsock.h>
//#include <WinBase.h>
//#include <lmcons.h>
#include "getoptwin.h"

#else
#include <unistd.h> // gethostname, getlogin_r

#include <getopt.h>
#endif

#include <sys/stat.h> // Statistics for files
#include <filesystem>
#include <iostream>

#include "printbuildinfo.h"

#include "mesh.h"

using namespace std;

bool cleanupGigaMeshData(
                const  std::string& fileNameIn,
                const  std::string& fileNameOutSuffix,
                const  std::string& rMaterialWhenEmpty,
                bool                rMaterialWhenEmptySet,
                bool                rFileNameAsIdWhenEmpty,
                double          percentArea = 0.1,
                bool            applyBorderErosion = true,
                bool            replaceFiles = false,
                bool            removeOnlyFlag = false,
                bool            keepLargestComponent = false,
                bool            skipLargestHole = false,
                unsigned long   maxNumberVertices = 3000
) {
	// Check file extension for input file
	size_t foundDot = fileNameIn.rfind(".");
	if( foundDot == std::string::npos ) {
		std::cerr << "[GigaMesh] ERROR: No extension/type for input file '" << fileNameIn << "' specified!" << std::endl;
		return( false );
	}

#ifndef WIN32
	// Check: Input file exists using file statistics
	struct stat stFileInfo;
	if( stat( fileNameIn.c_str(), &stFileInfo ) != 0 ) {
		std::cerr << "[GigaMesh] ERROR: Input file '" << fileNameIn << "' not found!" << std::endl;
		return( false );
	}

	// Check fileprefix for output - when not given use the name of the input file
	std::string fileNameOut( std::filesystem::path( fileNameIn ).stem() );
	fileNameOut += fileNameOutSuffix + ".ply";
	if( stat( fileNameOut.c_str(), &stFileInfo ) == 0 ) {
		if(!replaceFiles) {
			std::cerr << "[GigaMesh] File '" << fileNameOut << "' already exists - SKIPPED!" << std::endl;
			return( true );
		}
		std::cerr << "[GigaMesh] Warning: File '" << fileNameOut << "' will be replaced!" << std::endl;
	}
#else
	// Check: Input file exists using file statistics
	struct _stat64 stFileInfo;
	if( _stat64( fileNameIn.c_str(), &stFileInfo ) != 0 ) {
		std::cerr << "[GigaMesh] ERROR: Input file '" << fileNameIn << "' not found!" << std::endl;
		return( false );
	}

	// Check fileprefix for output - when not given use the name of the input file
	std::string fileNameOut( std::filesystem::path( fileNameIn ).stem().string() );
	fileNameOut += fileNameOutSuffix + ".ply";
	if( _stat64( fileNameOut.c_str(), &stFileInfo ) == 0 ) {
		if(!replaceFiles) {
			std::cerr << "[GigaMesh] File '" << fileNameOut << "' already exists - SKIPPED!" << std::endl;
			return( true );
		}
		std::cerr << "[GigaMesh] Warning: File '" << fileNameOut << "' will be replaced!" << std::endl;
	}
#endif

	// Filenames without path
	std::string fileNameInName  = std::filesystem::path( fileNameIn ).filename().string();
	std::string fileNameOutName = std::filesystem::path( fileNameOut ).filename().string();

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// PREPARE data structures and Meta-data
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	bool readSucess;
	Mesh someMesh( fileNameIn, readSucess );
	if( !readSucess ) {
		std::cerr << "[GigaMesh] Error: Could not open file '" << fileNameIn << "'!" << std::endl;
		return false;
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Meta-data options:
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if( rMaterialWhenEmptySet ) {
		std::string currMat = someMesh.getModelMetaString( MeshIO::META_MODEL_MATERIAL );
		if( currMat.size() == 0 ) {
			someMesh.setModelMetaString( MeshIO::META_MODEL_MATERIAL, rMaterialWhenEmpty );
		}
	}
	if( rFileNameAsIdWhenEmpty ) {
		std::string currModelId = someMesh.getModelMetaString( MeshIO::META_MODEL_ID );
		if( currModelId.size() == 0 ) {
			currModelId = std::filesystem::path( fileNameInName ).stem().string();
			std::replace( currModelId.begin(), currModelId.end(), '_', ' ' );
			someMesh.setModelMetaString( MeshIO::META_MODEL_ID, currModelId );
		}
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// + MAIN FUNCTIONS
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// Measure comput time
	//----------------------------------------------------------
	std::chrono::system_clock::time_point tStart = std::chrono::system_clock::now();

	// Keep ONLY the largest connected component.
	//----------------------------------------------------------
	if( !keepLargestComponent ) {
		uint64_t largestLabelId;
		std::set<long> labelNrs;

		// Determine connected components and select the largest
		//----------------------------------------------------------
		someMesh.labelVerticesAll();
		someMesh.getVertLabelAreaLargest( largestLabelId );

		// Select and remove everything except the largest component
		//----------------------------------------------------------
		std::cout << "[GigaMesh] Label kept: " << largestLabelId << std::endl;
		labelNrs.insert( -static_cast<int64_t>(largestLabelId) );

		someMesh.deSelMVertsAll();
		someMesh.selectVertLabelNo( labelNrs );
		someMesh.removeVerticesSelected();
	}

	// Initial numbers of primitives
	unsigned long oldVertexNr = someMesh.getVertexNr();
	unsigned long oldFaceNr = someMesh.getFaceNr();

	// MAIN cleaning procedure
	//----------------------------------------------------------
	if( removeOnlyFlag ) {
		// Single iteration of automatic mesh polishing, BECAUSE
		someMesh.removeUncleanSmall( percentArea, applyBorderErosion, "" ); // use fileNameOut3D instead of "" for saving the intermediate mesh.

		std::cout << "[GigaMesh] REMOVE ONLY: " << oldVertexNr-someMesh.getVertexNr() << " Vertices and ";
		std::cout << oldFaceNr-someMesh.getFaceNr() << " Faces removed." << std::endl;
	} else {
		//! \bug Check why completeRestore is unstable without intermediate storage.
		// BROKEN: Apply automatic mesh polishing using the corresponding method
		// someMesh.completeRestore( "", percentArea, applyBorderErosion, skipLargestHole, maxNumberVertices, nullptr ); // use fileNameOut instead of "" for saving the intermediate mesh.
		someMesh.completeRestore( fileNameOut, percentArea, applyBorderErosion, skipLargestHole, maxNumberVertices, nullptr ); // use fileNameOut instead of "" for saving the intermediate mesh.

		std::cout << "[GigaMesh] POLISH: Vertex count changed by " << static_cast<long>(someMesh.getVertexNr())-static_cast<long>(oldVertexNr) << std::endl;
		std::cout << "[GigaMesh]         Face count changed by   " << static_cast<long>(someMesh.getFaceNr())-static_cast<long>(oldFaceNr) << std::endl;

		// Prepare for editing holes i.e. false-positiv filled connected components
		//-------------------------------------------------------------------------
		someMesh.selVertByFlag( Primitive::FLAG_SYNTHETIC );
		someMesh.labelSelectedVertices();
	}

	std::chrono::system_clock::time_point tEnd = std::chrono::system_clock::now();
	//std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>( tEnd - tStart );
	double time_elapsed = ( std::chrono::duration<double>( tEnd - tStart ) ).count();
	time_t startTime = std::chrono::system_clock::to_time_t( tStart );
	time_t endTime   = std::chrono::system_clock::to_time_t( tEnd );

	// WRITE clean data
	//----------------------------------------------------------
	if( !someMesh.writeFile( fileNameOut ) ) {
		std::cerr << "[GigaMesh] ERROR: writing file " << fileNameOut << "!" << std::endl;
		return( false );
	}

	// WRITE meta information
	//----------------------------------------------------------
	// Count primitives and their properties for meta information
	MeshInfoData rFileInfos;
	someMesh.getMeshInfoData( rFileInfos, true );
	// Set filename for meta-data
	std::string fileNameOutMeta( std::filesystem::path( fileNameIn ).stem().string() );
	fileNameOutMeta += fileNameOutSuffix + ".info-clean.txt";
	// Open file for meta-data
	fstream fileStrOutMeta;
	fileStrOutMeta.open( fileNameOutMeta.c_str(), fstream::out );
#ifdef VERSION_PACKAGE
	fileStrOutMeta << "[GigaMesh] CLEAN v." << VERSION_PACKAGE << endl;
#else
	fileStrOutMeta << "[GigaMesh] CLEAN unknown version"< endl;
#endif
	fileStrOutMeta << "File Input:                 " << fileNameInName << endl;
	fileStrOutMeta << "File Output:                " << fileNameOutName << endl;
	fileStrOutMeta << "Model Id:                   " << rFileInfos.mStrings[MeshInfoData::MODEL_ID] << endl;
	fileStrOutMeta << "Model Material:             " << rFileInfos.mStrings[MeshInfoData::MODEL_MATERIAL] << endl;
	fileStrOutMeta << "Web-Reference:              " << rFileInfos.mStrings[MeshInfoData::MODEL_WEBREFERENCE] << std::endl;
	fileStrOutMeta << "Vertex count (in, out):     " << oldVertexNr << ", " << rFileInfos.mCountULong[MeshInfoData::VERTICES_TOTAL] << endl;
	fileStrOutMeta << "Face count (in, out):       " << oldFaceNr   << ", " << rFileInfos.mCountULong[MeshInfoData::FACES_TOTAL]    << endl;
	fileStrOutMeta << "Vertex count (diff):        " << ( static_cast<long>(rFileInfos.mCountULong[MeshInfoData::VERTICES_TOTAL]) - static_cast<long>(oldVertexNr) ) << endl;
	fileStrOutMeta << "Face count (diff):          " << ( static_cast<long>(rFileInfos.mCountULong[MeshInfoData::FACES_TOTAL])    - static_cast<long>(oldFaceNr)   ) << endl;
	fileStrOutMeta << "Vertices at border (out):   " << rFileInfos.mCountULong[MeshInfoData::VERTICES_BORDER] << endl;
	fileStrOutMeta << "Vertices synthetic (out):   " << rFileInfos.mCountULong[MeshInfoData::VERTICES_SYNTHETIC] << endl;
	fileStrOutMeta << "Faces synthetic (out):      " << rFileInfos.mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES] << endl;
	fileStrOutMeta << "Start time:                 " << std::ctime( &startTime );
	fileStrOutMeta << "Finish time:                " << std::ctime( &endTime );
	fileStrOutMeta << "Timespan (sec):             " << time_elapsed << endl;
	fileStrOutMeta.close();

	// Done.
	//----------------------------------------------------------
	return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << endl;
	std::cout << "GigaMesh Software Framework CLEAN 3D-data" << endl << endl;
	std::cout << "Cleans given meshes. The output file(s) will be namend the same as the input file " << endl;
	std::cout << "using the suffix GMC[F]:" << endl;
	std::cout << endl;
	std::cout << "  GM ... processed with GigaMesh" << endl;
	std::cout << "  C .... cleaned i.e. faulty parts removed" << endl;
	std::cout << "  F .... holes filled unless -r is used" << endl;
	std::cout << endl;
	std::cout << "Options:" << endl;
	std::cout << "  -h, --help                              Displays this help." << endl;
	std::cout << "  -v, --version                           Displays version information." << endl;
	std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is not done by default" << endl;
	std::cout << "                                          to prevent accidental data loss. If this option is not set" << endl;
	std::cout << "                                          and the output file exists, the file will be skipped." << endl;
	std::cout << endl;
	std::cout << "Options for cleaning and polishing:" << endl;
	std::cout << "  -r, --remove-only                       No holes are filled. Only corrupt mesh parts are removed." << endl;
	std::cout << "                                          File suffix is set to GMC. Options -s and -g will have no effect." << endl;
	std::cout << "  -p, --remove-components-relative SIZE   Remove all connected components i.e. surface parts smaller than" << endl;
	std::cout << "                                          the given percentage [0.0...1.0]. The default for SIZE is 0.1." << endl;
	std::cout << "  -l, --keep-largest-component            Keep only the largest connected component." << endl;
	std::cout << "  -s, --skip-largest-hole                 Do not fill the largest hole. Has no effect, when -r is used." << endl;
	std::cout << "                                          Typically used for partial 3D-scans, where e.g. the backside is missing." << endl;
	std::cout << "  -g, --skip-holes-larger SIZE            Holes with more than SIZE vertices are not filled. Has no effect, when -r is used." << endl;
	std::cout << "                                          Typically used for surface with large holes, which often have a complex shape." << endl;
	std::cout << "                                          The default for SIZE is 3000. Set 0 (zero) to attempted all holes to be filled." << endl;
	std::cout << "  -n, --no-border-erosion                 Do not apply border erosion i.e. keep dangling faces along the border." << endl;
	std::cout << endl;
	std::cout << "Options to (pre)set the embedded Meta-data:" << endl;
	std::cout << "  -m, --set-material-when-empty STRING    Set the material to STRING, when empty." << endl;
	std::cout << "  -i, --set-id-when-empty                 Set the stem of the <file> as id, when empty." << endl;
	//std::cout << "" << endl;
}


//! Main routine for loading a (binary) PLY and store it after cleaning the mesh.
//==============================================================================================================================================================

int main( int argc, char* argv[] ) {

	// Default string parameter
	std::string fileNameOutSuffix = "_GMCF";
	std::string materialWhenEmpty = "n.a.";

	// Default integer parameter
	unsigned long skipHolesLargerThan = 3000;

	// Default float parameter
	double percentArea = 0.1;

	// Default flags
	bool replaceFiles = false;
	bool removeOnlyFlag = false;
	bool keepLargestComponent = false;
	bool skipLargestHole = false;
	bool applyBorderErosion = true;
	bool materialWhenEmptySet = false;
	bool fileNameAsIdWhenEmpty = false;

	// PARSE command line options
	//--------------------------------------------------------------------------
	// https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
	static struct option longOptions[] = {
		{ "overwrite-existing",           no_argument,       nullptr, 'k' },
		{ "remove-only",                  no_argument,       nullptr, 'r' },
		{ "remove-components-relative",   required_argument, nullptr, 'p' },
		{ "keep-largest-component",       no_argument,       nullptr, 'l' },
		{ "skip-largest-hole",            no_argument,       nullptr, 's' },
		{ "skip-holes-larger",            required_argument, nullptr, 'g' },
		{ "no-border-erosion",            no_argument,       nullptr, 'n' },
		{ "set-material-when-empty",      required_argument, nullptr, 'm' },
		{ "set-id-when-empty",            required_argument, nullptr, 'i' },
		{ "version",                      no_argument,       nullptr, 'v' },
		{ "help",                         no_argument,       nullptr, 'h' },
		{ nullptr, 0, nullptr, 0 }
	};

	int character = 0;
	int optionIndex = 0;

	while( ( character = getopt_long_only( argc, argv, ":krp:lsg:nm:ivh",
	         longOptions, &optionIndex ) ) != -1 ) {
		switch(character) {
			case 0:

				if( longOptions[optionIndex].flag == nullptr ) {
					break;
				}

				// printf ("option %s", long_options[option_index].name);
				// if (optarg) printf (" with arg %s", optarg);	
				break;

			case 'k': // replaces output files
				std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
				replaceFiles = true;
				break;

			case 'r':
				// --remove-only flag was set.
				removeOnlyFlag = true;
				fileNameOutSuffix = "_GMC";
				break;

			case 'p':
				percentArea = std::atof( optarg );
				break;

			case 'l':
				keepLargestComponent = true;
				break;

			case 's':
				skipLargestHole = true;
				break;

			case 'n':
				applyBorderErosion = false;
				break;

			case 'g':
				skipHolesLargerThan = std::stoul( optarg );
				break;

			case 'm':
				materialWhenEmpty = std::string( optarg );
				materialWhenEmptySet = true;
				break;

			case 'i':
				fileNameAsIdWhenEmpty = true;
				break;

			case 'v':
				std::cout << "GigaMesh Software Framework CLEAN 3D-data " << VERSION_PACKAGE << endl;
#ifdef THREADS
				std::cout << "Multi-threading with " << NUM_THREADS << " threads." << endl;
#else
				std::cout << "Single-threading. " << endl;
#endif
#ifdef LIBPSALM
				std::cout << "Hole filling enabled. POLISH possible." << endl;
#else
				std::cout << "Hole filling disabled. CLEAN only." << endl;
#endif
				std::exit( EXIT_SUCCESS );
				break;

			case 'h':
				printHelp( argv[0] );
				std::exit( EXIT_SUCCESS );
				break;

			default: // Unknown option given
				std::cerr << "[GigaMesh] ERROR: Unknown option '" << character << "'!" << std::endl;
				std::cerr << "[GigaMesh]        See -h or --help for available options." << std::endl;
				std::exit( EXIT_FAILURE );
		}
	}

	// No files given i.e. wrong arguments
	if( argc-optind <= 0 ) {
		std::cerr << "[GigaMesh] ERROR: No files given!" << std::endl << std::endl;
		printHelp( argv[0] );
		std::exit( EXIT_FAILURE );
	}

	// Show build info
	printBuildInfo();

	// Show parameters and flags
	//! \todo add/show ALL parameters and flags used.
	std::cout << "[GigaMesh] Holes consisting of more than " << skipHolesLargerThan << " vertices/edges are not attempted to be filled." << std::endl;
	std::cout << "[GigaMesh] Surfaces with an area less than " << ( percentArea*100.0 ) << "% of the total surface will be removed." << std::endl;
	std::cout << "[GigaMesh] ----------------------------------------------------------------------------------" << std::endl;

	// Process given files
	unsigned long filesProcessed = 0;
	for( int nonOptionArgumentCount = optind;
	     nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

		std::string nonOptionArgumentString = std::string( argv[nonOptionArgumentCount] );

		if( !nonOptionArgumentString.empty() ) {
			std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

			if( !cleanupGigaMeshData( nonOptionArgumentString, fileNameOutSuffix,
			                          materialWhenEmpty, materialWhenEmptySet, fileNameAsIdWhenEmpty,
			    percentArea, applyBorderErosion, replaceFiles, removeOnlyFlag,
			    keepLargestComponent, skipLargestHole,
			    skipHolesLargerThan ) ) {
				std::cerr << "[GigaMesh] ERROR: cleanupGigaMeshData failed!" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			filesProcessed++;
		}
	}

	std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
	std::exit( EXIT_SUCCESS );
}
