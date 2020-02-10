#include <stdio.h>
#include <stdlib.h> // calloc
#include <string>
#ifdef WIN32	//windows version for hostname and login
//#include <winsock.h>
//#include <WinBase.h>
//#include <lmcons.h>
#include "getoptwin.h"

#else
#include <unistd.h> // gethostname, getlogin_r

#include <getopt.h>
#endif
#include <filesystem>


#include <GigaMesh/printbuildinfo.h>
#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/logging/Logging.h>
//#include "meshseed.h"

// //#include "voxelcuboid.h"
//#include "voxelfilter25d.h"

#include <sys/stat.h> // statistics for files

using namespace std;

bool convertMeshData(
                const string&   rFileName,
                const string&   rFileSuffix,
                const bool      rWriteBinary,
                const bool      rWriteNormals,
                const bool      rReplaceFiles
) {
	std::string fileExtension = std::filesystem::path( rFileName ).extension().string();
	// Check file extension for input file
	if( fileExtension.size() != 4 ) {
		cerr << "[GigaMesh] ERROR: File extension '" << fileExtension << "' is faulty!" << endl;
		return( false );
	}

	// Add parameters to output prefix
	std::string fileNameOut = std::filesystem::path( rFileName ).stem().string();
	fileNameOut += rFileSuffix;

#ifndef WIN32
	// Check files using file statistics
	struct stat stFileInfo;
	// Check: Input file exists?
	if( stat( rFileName.c_str(), &stFileInfo ) != 0 ) {
		cerr << "[GigaMesh] Error: File '" << rFileName << "' not found!" << endl;
		return( false );
	}

	// Output file for 3D data including the volumetric feature vectors.
	string fileNameOut3D( fileNameOut );
	fileNameOut3D += ".ply";
	if( stat( fileNameOut3D.c_str(), &stFileInfo ) == 0 ) {
		if( !rReplaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOut3D << "' already exists!" << endl;
			return( false );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!" << endl;
	}
#else
	// Check files using file statistics
	struct _stat64 stFileInfo;
	// Check: Input file exists?
	if( _stat64( rFileName.c_str(), &stFileInfo ) != 0 ) {
		cerr << "[GigaMesh] Error: File '" << rFileName << "' not found!" << endl;
		return( false );
	}

	// Output file for 3D data including the volumetric feature vectors.
	string fileNameOut3D( fileNameOut );
	fileNameOut3D += ".ply";
	if( _stat64( fileNameOut3D.c_str(), &stFileInfo ) == 0 ) {
		if( !rReplaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOut3D << "' already exists!" << endl;
			return( false );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!" << endl;
	}
#endif
	// All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
	cout << "[GigaMesh] File IN:         " << rFileName << endl;
	cout << "[GigaMesh] File OUT/Prefix: " << fileNameOut << endl;

	// Prepare data structures
	//--------------------------------------------------------------------------
	bool readSucess;
	Mesh someMesh( rFileName, readSucess );
	if( !readSucess ) {
		cerr << "[GigaMesh] Error: Could not open file '" << rFileName << "'!" << endl;
		return( false );
	}

	// Fetch mesh data
	Vector3D bbDim;
	someMesh.getBoundingBoxSize( bbDim );
	double bbWdith  = (double)round( bbDim.getX()*1.0 ) / 10.0;
	double bbHeight = (double)round( bbDim.getY()*1.0 ) / 10.0;
	double bbThick  = (double)round( bbDim.getZ()*1.0 ) / 10.0;
	// Area and average resolution
	double areaAcq;
	someMesh.getFaceSurfSum( &areaAcq );
	areaAcq = round( areaAcq );
	double volDXYZ[3]{ 0.0, 0.0, 0.0 };
	someMesh.getMeshVolumeDivergence( volDXYZ[0], volDXYZ[1], volDXYZ[2] );
	string modelID  = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	string modelMat = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	// Write data to file
	cout << "[GigaMesh] Model ID:        " << modelID << endl;
	cout << "[GigaMesh] Material:        " << modelMat << endl;
	cout << "[GigaMesh] Vertices:        " << someMesh.getVertexNr() << "" << endl;
	cout << "[GigaMesh] Faces:           " << someMesh.getFaceNr() << "" << endl;
	cout << "[GigaMesh] Bounding Box:    " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << endl;
	cout << "[GigaMesh] Area:            " << areaAcq/100.0 << " cm^2" << endl;
	cout << "[GigaMesh] Volume (dx):     " << volDXYZ[0]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dy):     " << volDXYZ[1]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dz):     " << volDXYZ[2]/1000.0 << " cm^3" << endl;

	time_t     rawtime;
	struct tm* timeinfo;
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] Start date/time is: " << asctime( timeinfo );// << endl;
	someMesh.setFlagExport( MeshIO::EXPORT_BINARY,        rWriteBinary );
	someMesh.setFlagExport( MeshIO::EXPORT_VERT_NORMAL,   rWriteNormals );
	someMesh.setFlagExport( MeshIO::EXPORT_VERT_FLAGS, false );
	someMesh.setFlagExport( MeshIO::EXPORT_VERT_LABEL, false );
	someMesh.setFlagExport( MeshIO::EXPORT_VERT_FTVEC, false );
	someMesh.setFlagExport( MeshIO::EXPORT_POLYLINE,   false );
	someMesh.writeFile( fileNameOut3D );
	cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;

	return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
	std::cout << "GigaMesh Software Framework TO.LEGACY 3D-data" << std::endl << std::endl;
	std::cout << "Converts the given meshes to legacy Stanford Polygon Files (PLYs) i.e. strips extra information added by GigaMesh like feature vectors." << std::endl;
	std::cout << "Function values will be kept as quality field, which is widely accepted. ";
	std::cout << "Normals per vertex are not written per default as they are typically estimated for measurment data.";
	std::cout << std::endl << std::endl;
	std::cout << "Options:" << endl;
	std::cout << "  -h, --help                              Displays this help." << std::endl;
	std::cout << "  -v, --version                           Displays version information." << std::endl << std::endl;
	std::cout << "  -b, --binary                            Write the converted file binary." << std::endl;
	std::cout << "  -n, --write-normals                     Write the normal vectors per vertex." << std::endl;
	std::cout << "  -s, --output-suffix <string>            Write the converted file using the given <string> as suffix for its name." << std::endl;
	std::cout << "                                          Default suffices are '_ASCII' and '_Legacy'." << std::endl;
	std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is not done by default" << std::endl;
	std::cout << "                                          to prevent accidental data loss." << std::endl;
	std::cout << "    , --log-level [0-4]                   Sets the log level of this application.\n"
				 "                                          Higher numbers increases verbosity.\n"
				 "                                          (Default: 1)" << std::endl;
	//std::cout << "" << endl;
}

//! Main routine for loading a (binary) PLY and store it as ASCII without the extra data supplied by GigaMesh
//==============================================================================================================================================================
int main( int argc, char *argv[] ) {

	LOG::initLogging();

	// Default string parameter
	std::string optFileSuffix;

	// Default flags
	bool optReplaceFiles = false;
	bool optWriteBinary  = false;
	bool optWriteNormals = false;

	// PARSE command line options
	//--------------------------------------------------------------------------
	// https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
	static struct option longOptions[] = {
		{ "output-suffix",                required_argument, nullptr, 's' },
		{ "binary",                       no_argument,       nullptr, 'b' },
		{ "write-normals",                no_argument,       nullptr, 'n' },
		{ "overwrite-existing",           no_argument,       nullptr, 'k' },
		{ "version",                      no_argument,       nullptr, 'v' },
		{ "help",                         no_argument,       nullptr, 'h' },
		{ "log-level",                    required_argument, nullptr,  0  },
		{ nullptr, 0, nullptr, 0 }
	};

	int character = 0;
	int optionIndex = 0;

	while( ( character = getopt_long_only( argc, argv, ":s:bnkvh",
	         longOptions, &optionIndex ) ) != -1 ) {
		switch(character) {
			case 0:
				// printf ("option %s", long_options[option_index].name);
				// if (optarg) printf (" with arg %s", optarg);

				if(longOptions[optionIndex].name == "log-level")
				{
					unsigned int arg = optarg[0] - '0';
					if(arg <= 5)
					{
						LOG::setLogLevel(static_cast<LOG::LogLevel>(arg));
					}
					else
					{
						std::cerr << "[GigaMesh] WARNING: Log level is out of range [0-4]!" << std::endl;
					}
				}

				break;

			case 's':
				optFileSuffix = std::string( optarg );
				break;

			case 'b': // write binray file
				optWriteBinary = true;
				break;

			case 'n': // write vertex normals
				optWriteNormals = true;
				break;

			case 'k': // replaces output files
				std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
				optReplaceFiles = true;
				break;

			case 'v':
				std::cout << "GigaMesh Software Framework TO.LEGACY Stanford Polygon Files (PLYs) " << VERSION_PACKAGE << endl;
#ifdef THREADS
				std::cout << "Multi-threading with " << NUM_THREADS << " threads." << endl;
#else
				std::cout << "Single-threading. " << endl;
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

	// Add default suffix
	if( optFileSuffix.size() == 0 ) {
		optFileSuffix = "_ASCII";
		if( optWriteBinary ) {
			optFileSuffix = "_Legacy";
		}
	}

	// SHOW Build information
	printBuildInfo();

	// Process given files
	unsigned long filesProcessed = 0;
	for( int nonOptionArgumentCount = optind;
	     nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

		std::string nonOptionArgumentString = std::string( argv[nonOptionArgumentCount] );

		if( !nonOptionArgumentString.empty() ) {
			std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

			if( !convertMeshData( nonOptionArgumentString, optFileSuffix,
			                      optWriteBinary, optWriteNormals, optReplaceFiles ) ) {
				std::cerr << "[GigaMesh] ERROR: convertMeshData failed!" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			filesProcessed++;
		}
	}

	std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
	exit( EXIT_SUCCESS );
}
