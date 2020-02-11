#include <string>
#include <thread>
#include <future>
#include <mutex>
#include <functional>

#include <ctime>
#include <cstdio>
#include <cstdlib> // calloc

#ifdef _MSC_VER	//windows version for hostname and login
#include <winsock.h>
#include <WinBase.h>
#include <lmcons.h>
#include "getoptwin.h"
#elif defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#include <lmcons.h>
#include "getoptwin.h"
#else
#include <unistd.h> // gethostname, getlogin_r

#include <getopt.h>
#endif
#include <GigaMesh/printbuildinfo.h>
#include <GigaMesh/mesh/mesh.h>

//#include "voxelcuboid.h"
#include <GigaMesh/mesh/voxelfilter25d.h>

#include <sys/stat.h> // statistics for files
#include <GigaMesh/logging/Logging.h>

using namespace std;

#define _DEFAULT_FEATUREGEN_RADIUS_       1.0
#define _DEFAULT_FEATUREGEN_XYZDIM_       256
#define _DEFAULT_FEATUREGEN_RADIICOUNT_   4 // power of 2

	// Multithreading (CPU):
	// see: https://computing.llnl.gov/tutorials/pthreads/

	#define THREADS_VERTEX_BLOCK  5000
    std::mutex stdoutMutex;

	struct meshDataStruct {
		int     threadID{0}; //!< ID of the posix thread
		// input:
		Mesh*   meshToAnalyze{nullptr};
		double  radius{0.0};
		uint    xyzDim{0};
		uint    multiscaleRadiiSize{0};
		double* multiscaleRadii{nullptr};
		uint64_t*  vertexOriIdxInProgress{nullptr};
		uint64_t   nrOfVerticesToProcess{0};
		// output:
		int     ctrIgnored{0};
		int     ctrProcessed{0};
		// our most precious feature vectors (as array):
		double* patchNormal{nullptr};     //!< Normal used for orientation into 2.5D representation
		double* descriptVolume{nullptr};  //!< Volume descriptors
		double* descriptSurface{nullptr}; //!< Surface descriptors
		// and the original vertex indicies
		int*    featureIndicies{nullptr};
		voxelFilter2DElements** sparseFilters{nullptr};
	};

	void estFeatureVectors( meshDataStruct* meshData,
	                                const size_t threadOffset,
	                                const size_t threadVertexCount ) {

		const int threadID = meshData->threadID;

		{
			std::lock_guard<std::mutex> lock(stdoutMutex);
			cout << "[GigaMesh] Thread " << threadID  << " started, processing "
			        << threadVertexCount << " of vertices ("
			        << static_cast<double>(threadVertexCount)/
			            static_cast<double>(meshData->meshToAnalyze->getVertexNr())*100.0
			        << " % of total)" << endl;
		}

		// initalize values to be returned via meshDataStruct:
		meshData->ctrIgnored   = 0;
		meshData->ctrProcessed = 0;

		// copy pointers from struct for easier access.
		double* tDescriptVolume  = meshData->descriptVolume;  //!< Volume descriptors
		double* tDescriptSurface = meshData->descriptSurface; //!< Surface descriptors

		// setup memory for rastered surface:
		const uint    rasterSize = meshData->xyzDim * meshData->xyzDim;
		double* rasterArray = new double[rasterSize];
		const uint64_t  nrOfVerticesToProcess = meshData->nrOfVerticesToProcess;

		// Processing time
		std::chrono::system_clock::time_point tStart = std::chrono::system_clock::now();

		// Allocate the bit arrays for vertices:
		uint64_t* vertBitArrayVisited{nullptr};
		const uint64_t vertNrLongs = meshData->meshToAnalyze->getBitArrayVerts( &vertBitArrayVisited );
		// Allocate the bit arrays for faces:
		uint64_t* faceBitArrayVisited{nullptr};
		const uint64_t faceNrLongs = meshData->meshToAnalyze->getBitArrayFaces( &faceBitArrayVisited );

		// Compute absolut radii (used to normalize the surface descriptor)
		double* absolutRadii = new double[meshData->multiscaleRadiiSize];
		for( uint i=0; i<meshData->multiscaleRadiiSize; i++ ) {
			absolutRadii[i] = static_cast<double>(meshData->multiscaleRadii[i]) *
			                  static_cast<double>(meshData->radius);
		}

		// Step thru vertices:
		Vertex*       currentVertex{nullptr};
		vector<Face*> facesInSphere; // temp variable to store local surface patches - returned by fetchSphereMarching

		for( size_t vertexOriIdxInProgress = threadOffset;
		                vertexOriIdxInProgress < (threadOffset + threadVertexCount);
		                                                    ++vertexOriIdxInProgress )
		{

			currentVertex = meshData->meshToAnalyze->getVertexPos( vertexOriIdxInProgress );

			// Fetch faces within the largest sphere
			facesInSphere.clear();
			// slower for larger patches:
			//meshData->meshToAnalyze->fetchSphereMarching( currentVertex, &facesInSphere, meshData->radius, true );
			//meshData->meshToAnalyze->fetchSphereMarchingDualFront( currentVertex, &facesInSphere, meshData->radius, true );
			//meshData->meshToAnalyze->fetchSphereBitArray( currentVertex, &facesInSphere, meshData->radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );
			meshData->meshToAnalyze->fetchSphereBitArray1R( currentVertex, facesInSphere, meshData->radius, vertNrLongs,
			                                                    vertBitArrayVisited, faceNrLongs, faceBitArrayVisited, false );

			// Fetch and store the normal used in fetchSphereCubeVolume25D as it is a quality measure
			vector<Face*>::iterator itFace;
			for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
				(*itFace)->addNormalTo( &(meshData->patchNormal[vertexOriIdxInProgress*3]) );
			}

			// Pre-compute address offset
			unsigned int descriptIndexOffset = vertexOriIdxInProgress*meshData->multiscaleRadiiSize;

			// Get volume descriptor:
			if( tDescriptVolume ) {
				meshData->meshToAnalyze->fetchSphereCubeVolume25D( currentVertex, &facesInSphere,
				                                                   meshData->radius, rasterArray,
				                                                   meshData->xyzDim );
				applyVoxelFilters2D( &(tDescriptVolume[descriptIndexOffset]), rasterArray,
				                        meshData->sparseFilters, meshData->multiscaleRadiiSize, meshData->xyzDim );
			}

			// Get surface descriptor:
			if( tDescriptSurface ) {
				// Vector3D seedPosition = currentVertex->getCenterOfGravity();
				meshData->meshToAnalyze->fetchSphereArea( currentVertex, &facesInSphere,
				                                          static_cast<unsigned int>(meshData->multiscaleRadiiSize),
				                                          absolutRadii, &(tDescriptSurface[descriptIndexOffset]) );
			}
			// Set counters:
			meshData->ctrProcessed++;

			if(  !( ( vertexOriIdxInProgress - threadOffset ) % THREADS_VERTEX_BLOCK ) &&
			                                                        (vertexOriIdxInProgress - threadOffset) ) {
				// Show a time estimation:

				const double percentDone = static_cast<double>(vertexOriIdxInProgress - threadOffset) /
				                                                    static_cast<double>(threadVertexCount);

				std::chrono::system_clock::time_point tEnd = std::chrono::system_clock::now();
				//std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>( tEnd - tStart );
				double time_elapsed = ( std::chrono::duration<double>( tEnd - tStart ) ).count();
				double time_remaining =   (time_elapsed * (1.0/percentDone)) - time_elapsed;
				std::chrono::system_clock::time_point tFinalEst = tEnd + std::chrono::seconds( static_cast<long>( time_remaining ) );
				std::time_t ttp = std::chrono::system_clock::to_time_t(tFinalEst);

				{
					std::lock_guard<std::mutex> lock(stdoutMutex);

					cout << "[GigaMesh] Thread " << threadID << " | " << percentDone*100 << " percent done. Time elapsed: " << time_elapsed << " - ";
					cout << "remaining: " << time_remaining << " seconds. ";
					cout << vertexOriIdxInProgress/time_elapsed << " Vert/sec. ";
					cout << "ETF: " << std::ctime(&ttp);
					cout << std::flush;
				}
			}
		}

		// Volume descriptor
		delete[] rasterArray;
		delete vertBitArrayVisited;
		delete faceBitArrayVisited;
		// Surface descriptor
		delete[] absolutRadii;

		{
			std::lock_guard<std::mutex> lock(stdoutMutex);
			cout << "[GigaMesh] Thread " << threadID << " | STOP - processed: " << meshData->ctrProcessed << " and skipped " << meshData->ctrIgnored << " vertices." << endl;
		}
	}

//! Main routine for generating an array of feature vectors
//!
//! Remark: prefer MeshSeed over Mesh as it is faster by a factor of 3+
//==========================================================================
int main( int argc, char *argv[] ) {

	LOG::initLogging();
	// SHOW Build information
	printBuildInfo();

	string       fileNameIn;
	string       fileNameOut;
	double       radius{_DEFAULT_FEATUREGEN_RADIUS_};
	unsigned int xyzDim{_DEFAULT_FEATUREGEN_XYZDIM_};
	unsigned int radiiCount{_DEFAULT_FEATUREGEN_RADIICOUNT_};
	bool         replaceFiles{false};
	bool         areaOnly{false};

	// PARSE command line options
	//--------------------------------------------------------------------------
	opterr = 0;
	int c{0};
	int tmpInt{0};
	bool radiusSet{false};
	while( ( c = getopt( argc, argv, "f:o:r:v:n:kal:" ) ) != -1 ) {
		switch( c ) {
			//! Option f: filename for input data (required)
			case 'f':
				fileNameIn = optarg;
				break;
			//! Option o: prefix for files with filter results and metadata (optional/automatic)
			case 'o':
				fileNameOut = optarg;
				break;
			//! Option r: absolut radius (in units, default: 1.0)
			case 'r':
				radius = atof( optarg );
				radiusSet = true;
				break;
			//! Option v: edge length of the voxel cube (in voxels, default: 256)
			case 'v':
				tmpInt = atof( optarg );
				if( tmpInt <= 0 ) {
					cerr << "[GigaMesh] Error: negative or zero value given: " << tmpInt << " for the number of voxels (option -v)!" << endl;
					exit( EXIT_FAILURE );
				}
				xyzDim = static_cast<unsigned int>(tmpInt);
				break;
			//! Option n: 2^n scales (default: 4 => 16 scales)
			case 'n':
				tmpInt = atof( optarg );
				if( tmpInt < 0 ) {
					cerr << "[GigaMesh] Error: negative value given: " << tmpInt << " for the number of radii (option -n)!" << endl;
					exit( EXIT_FAILURE );
				}
				radiiCount = static_cast<unsigned int>(tmpInt);
				break;
			//! Option k: replaces output files
			case 'k':
				cout << "[GigaMesh] Warning: files might be replaced!" << endl;
				replaceFiles = true;
				break;
			//! Option a: compute area/surface based integral onyl
			case 'a':
				cout << "[GigaMesh] Warning: Only area integrals will be computed!" << endl;
				areaOnly = true;
				break;
			case 'l':
				tmpInt = atoi( optarg );
				if(tmpInt < 0 || tmpInt > 4)
				{
					cerr << "[GigaMesh] Error: logging value not in range of [0-4]\n";
				}
				else
				{
					LOG::setLogLevel(static_cast<LOG::LogLevel>(tmpInt));
				}
			case '?':
				cerr << "[GigaMesh] Error: Unknown option!" << endl;
				break;
			default:
				cerr << "[GigaMesh] Error: Unknown option '" << c << "'!" << endl;
				exit( EXIT_FAILURE );
		}
	}
	// Check argument ranges
	if( radius <= 0.0 ) {
		cerr << "[GigaMesh] Error: negative or zero radius given: " << radius << " (option -r)!" << endl;
		exit( EXIT_FAILURE );
	}
	if( !radiusSet ) {
		cout << "[GigaMesh] Warning: default radius is used (option -r missing)!" << endl;
	}
	if( fileNameIn.length() == 0 ) {
		cerr << "[GigaMesh] Error: no filename for input given (option -f)!" << endl;
		exit( EXIT_FAILURE );
	}
	// Check file extension for input file
	size_t foundDot = fileNameIn.rfind( "." );
	if( foundDot == string::npos ) {
		cerr << "[GigaMesh] Error: No extension/type for input file '" << fileNameIn << "' specified!" << endl;
		exit( EXIT_FAILURE );
	}
	// Check fileprefix for output - when not given use the name of the input file
	if( fileNameOut.length() == 0 ) {
		fileNameOut = fileNameIn.substr( 0, foundDot );
		// Warning message see a few lines below.
	}
	// Add parameters to output prefix
	char tmpBuffer[512];
	sprintf( tmpBuffer, "_r%0.2f_n%i_v%i", radius, radiiCount, xyzDim );
	fileNameOut += string( tmpBuffer );
	// Warning message, for option -o missing
	if( fileNameOut.length() == 0 ) {
		cerr << "[GigaMesh] Warning: no prefix for output given (option -o) using: '" << fileNameOut << "'!" << endl;
	}
	// Check files using file statistics
	struct stat stFileInfo;
	// Check: Input file exists?
	if( stat( fileNameIn.c_str(), &stFileInfo ) != 0 ) {
		cerr << "[GigaMesh] Error: File '" << fileNameIn << "' not found!" << endl;
		exit( EXIT_FAILURE );
	}
	// Check: Output file for normal used to rotate the local patch
	string fileNameOutPatchNormal( fileNameOut );
	fileNameOutPatchNormal += ".normal.mat";
	if( stat( fileNameOutPatchNormal.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutPatchNormal << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutPatchNormal << "' will be replaced!" << endl;
	}
	string fileNameOutVS;
	string fileNameOutVol;
	if( !areaOnly ) {
		// Check: Output file for volume AND surface descriptor
		fileNameOutVS = fileNameOut;
		fileNameOutVS += ".vs.mat";
		if( stat( fileNameOutVS.c_str(), &stFileInfo ) == 0 ) {
			if( !replaceFiles ) {
				cerr << "[GigaMesh] File '" << fileNameOutVS << "' already exists!" << endl;
				exit( EXIT_FAILURE );
			}
			cerr << "[GigaMesh] Warning: File '" << fileNameOutVS << "' will be replaced!" << endl;
		}
		// Check: Output file for volume descriptor
		fileNameOutVol = fileNameOut;
		fileNameOutVol += ".volume.mat";
		if( stat( fileNameOutVol.c_str(), &stFileInfo ) == 0 ) {
			if( !replaceFiles ) {
				cerr << "[GigaMesh] File '" << fileNameOutVol << "' already exists!" << endl;
				exit( EXIT_FAILURE );
			}
			cerr << "[GigaMesh] Warning: File '" << fileNameOutVol << "' will be replaced!" << endl;
		}
	}
	// Output file for surface descriptor
	string fileNameOutSurf( fileNameOut );
	fileNameOutSurf += ".surface.mat";
	if( stat( fileNameOutSurf.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutSurf << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutSurf << "' will be replaced!" << endl;
	}
	// Output file for meta-information
	string fileNameOutMeta( fileNameOut );
	fileNameOutMeta += ".info.txt";
	if( stat( fileNameOutMeta.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutMeta << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutMeta << "' will be replaced!" << endl;
	}
	// Output file for 3D data including the volumetric feature vectors.
	string fileNameOut3D( fileNameOut );
	fileNameOut3D += ".ply";
	if( stat( fileNameOut3D.c_str(), &stFileInfo ) == 0 ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOut3D << "' already exists!" << endl;
			exit( EXIT_FAILURE );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!" << endl;
	}
	// Invalid/unexpected paramters
	for( int i = optind; i < argc; i++ ) {
		cerr << "[GigaMesh] Warning: Non-option argument '" << argv[i] << "' given and ignored!" << endl;
	}
	// All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
	fstream fileStrOutMeta;
	fileStrOutMeta.open( fileNameOutMeta.c_str(), fstream::out );
	cout << "[GigaMesh] File IN:         " << fileNameIn << endl;
	cout << "[GigaMesh] File OUT/Prefix: " << fileNameOut << endl;
	cout << "[GigaMesh] Radius:          " << radius << " mm (unit assumed)" << endl;
	cout << "[GigaMesh] Radii:           2^" << radiiCount << " = " << pow( 2.0, static_cast<double>(radiiCount) ) << endl;
	cout << "[GigaMesh] Rastersize:      " << xyzDim << "^3" << endl;
#ifdef VERSION_PACKAGE
	fileStrOutMeta << "GigaMesh Version    " << VERSION_PACKAGE << endl;
#else
	fileStrOutMeta << "GigaMesh Version    unknown" << endl;
#endif
#ifdef THREADS
	fileStrOutMeta << "Threads (fixed):    " << std::thread::hardware_concurrency() * 2 << endl;
#else
	fileStrOutMeta << "Threads (fixed):    single" << endl;
#endif
	fileStrOutMeta << "File IN:            " << fileNameIn << endl;
	fileStrOutMeta << "File OUT/Prefix:    " << fileNameOut << endl;
	fileStrOutMeta << "Radius:             " << radius << " mm (unit assumed)" << endl;
	fileStrOutMeta << "Radii:              2^" << radiiCount << " = " << std::pow( 2.0, static_cast<float>(radiiCount) ) << endl;
	fileStrOutMeta << "Rastersize:         " << xyzDim << "^3" << endl;

	// Compute relative radii:
	uint64_t multiscaleRadiiSize = std::pow( 2.0, static_cast<double>(radiiCount) );
	double*       multiscaleRadii     = new double[multiscaleRadiiSize];
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		multiscaleRadii[i] = 1.0 - static_cast<double>(i)/
		                            static_cast<double>(multiscaleRadiiSize);
	}

	// Set the formatting properties of the output
	cout << setprecision( 2 ) << fixed;
	fileStrOutMeta << setprecision( 2 ) << fixed;
	// Info about files
	cout << "[GigaMesh] File to write patch normal: " << fileNameOutPatchNormal << endl;
	if( !fileNameOutVS.empty() ) {
		cout << "[GigaMesh] File to write V+S:          " << fileNameOutVS << endl;
	}
	if( !fileNameOutVol.empty() ) {
		cout << "[GigaMesh] File to write Volume:       " << fileNameOutVol << endl;
	}
	if( !fileNameOutSurf.empty() ) {
		cout << "[GigaMesh] File to write Surface:      " << fileNameOutSurf << endl;
	}
	cout << "[GigaMesh] File to write Metainfo:     " << fileNameOutMeta << endl;
	cout << "[GigaMesh] Radii: (realtive)          ";
	fileStrOutMeta << "Radii (realtive):  ";
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		cout << " " << multiscaleRadii[i];
		fileStrOutMeta << " " << multiscaleRadii[i];
	}
	cout << endl;
	fileStrOutMeta << endl;
	fileStrOutMeta << "Radii (ansolute):  ";
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		fileStrOutMeta << " " << ( multiscaleRadii[i] * radius );
	}
	fileStrOutMeta << endl;

	// Prepare data structures
	//--------------------------------------------------------------------------
	bool readSucess;
	Mesh someMesh( fileNameIn, readSucess );
	if( !readSucess ) {
		cerr << "[GigaMesh] Error: Could not open file '" << fileNameIn << "'!" << endl;
		exit( EXIT_FAILURE );
	}

	// Fetch mesh data
	Vector3D bbDim;
	someMesh.getBoundingBoxSize( bbDim );
	const double bbWdith{static_cast<double>(std::round( bbDim.getX()*1.0 )) / 10.0};
	const double bbHeight{static_cast<double>(std::round( bbDim.getY()*1.0 )) / 10.0};
	const double bbThick{static_cast<double>(std::round( bbDim.getZ()*1.0 )) / 10.0};
	// Area and average resolution
	double areaAcq{0};
	someMesh.getFaceSurfSum( &areaAcq );
	areaAcq = round( areaAcq );
	double volDXYZ[3]{ 0.0, 0.0, 0.0 };
	someMesh.getMeshVolumeDivergence( volDXYZ[0], volDXYZ[1], volDXYZ[2] );
	string modelID = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	string modelMat = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	string modelWebRef = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_REFERENCE_WEB );
	// Write data to console
#ifdef VERSION_PACKAGE
	cout << "[GigaMesh] Version:         " << VERSION_PACKAGE << endl;
#else
	cout << "[GigaMesh] Version:         unknown" << endl;
#endif
#ifdef THREADS
	cout << "[GigaMesh] Threads:         " << std::thread::hardware_concurrency() * 2 << endl;
#else
	cout << "[GigaMesh] Threads:         single" << endl;
#endif
	cout << "[GigaMesh] ==================================================" << endl;
	cout << "[GigaMesh] Model ID:        " << modelID << endl;
	cout << "[GigaMesh] Material:        " << modelMat << endl;
	cout << "[GigaMesh] Web-reference:   " << modelWebRef << endl;
	cout << "[GigaMesh] --------------------------------------------------" << endl;
	cout << "[GigaMesh] Vertices:        " << someMesh.getVertexNr() << "" << endl;
	cout << "[GigaMesh] Faces:           " << someMesh.getFaceNr() << "" << endl;
	cout << "[GigaMesh] Bounding Box:    " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << endl;
	cout << "[GigaMesh] Area:            " << areaAcq/100.0 << " cm^2" << endl;
	cout << "[GigaMesh] Volume (dx):     " << volDXYZ[0]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dy):     " << volDXYZ[1]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dz):     " << volDXYZ[2]/1000.0 << " cm^3" << endl;
	// Write data to file
	fileStrOutMeta << "Model ID:           " << modelID << endl;
	fileStrOutMeta << "Material:           " << modelMat << endl;
	fileStrOutMeta << "Web-reference:      " << modelWebRef << endl;
	fileStrOutMeta << "Vertices:           " << someMesh.getVertexNr() << "" << endl;
	fileStrOutMeta << "Faces:              " << someMesh.getFaceNr() << "" << endl;
	fileStrOutMeta << "Bounding Box:       " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << endl;
	fileStrOutMeta << "Area:               " << areaAcq/100.0 << " cm^2" << endl;
	fileStrOutMeta << "Volume (dx):        " << volDXYZ[0]/1000.0 << " cm^3" << endl;
	fileStrOutMeta << "Volume (dy):        " << volDXYZ[1]/1000.0 << " cm^3" << endl;
	fileStrOutMeta << "Volume (dz):        " << volDXYZ[2]/1000.0 << " cm^3" << endl;

#ifdef WIN32
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char hostname[256] = {0};
	auto error = gethostname(hostname, 256);

	if(error != 0)
	{
		std::cerr << "Error, could not get hostname!" << std::endl;
	}
	char username[UNLEN- 1] = {0};
	DWORD len = UNLEN - 1;
	if(!GetUserNameA(username,&len))
	{
		std::cerr << "Error, could not get username!" << std::endl;
	}
	WSACleanup();
#else
	// Write hostname and username - see: https://stackoverflow.com/questions/27914311/get-computer-name-and-logged-user-name
	char hostname[HOST_NAME_MAX] = {0};
	char username[LOGIN_NAME_MAX] = {0};
	gethostname( hostname, HOST_NAME_MAX );
	auto error = getlogin_r( username, LOGIN_NAME_MAX );

	if(error != 0)
	{
		std::cerr << "Error, could not get username!" << std::endl;
	}
#endif

	fileStrOutMeta << "Hostname:           " << hostname << endl;
	fileStrOutMeta << "Username:           " << username << endl;

	int*    featureIndicies = new int[someMesh.getVertexNr()];
	double* patchNormal     = new double[someMesh.getVertexNr()*3];
	double* descriptVolume{nullptr};
	if( !areaOnly ) {
		descriptVolume  = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	}
	double* descriptSurface = NULL;
	descriptSurface = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
		featureIndicies[i] = i;
		patchNormal[i*3]   = 0.0;
		patchNormal[i*3+1] = 0.0;
		patchNormal[i*3+2] = 0.0;
	}
	if( descriptVolume != NULL ) {
		for( uint i=0; i<someMesh.getVertexNr()*multiscaleRadiiSize; i++ ) {
			descriptVolume[i]  = _NOT_A_NUMBER_DBL_;
		}
	}
	if( descriptSurface != NULL ) {
		for( uint i=0; i<someMesh.getVertexNr()*multiscaleRadiiSize; i++ ) {
			descriptSurface[i] = _NOT_A_NUMBER_DBL_;
		}
	}

	// Determine number of threads using CPU cores minus one.
	const unsigned int availableConcurrentThreads =  std::thread::hardware_concurrency() - 1;
	std::cout << "[GigaMesh] Computing feature vectors using "
	            << availableConcurrentThreads << " threads" << std::endl;
	fileStrOutMeta << "Threads (dynamic):  " << availableConcurrentThreads << endl;

	time_t rawtime;
	struct tm* timeinfo{nullptr};
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] Start date/time is: " << asctime( timeinfo );// << endl;
	fileStrOutMeta << "Start date/time is: " << asctime( timeinfo ); // no endl required as asctime will add a linebreak

	time_t timeStampParallel = time( nullptr ); // clock() is not multi-threading save (to measure the non-CPU or real time ;) )
	voxelFilter2DElements* sparseFilters;
	generateVoxelFilters2D( multiscaleRadiiSize, multiscaleRadii, xyzDim, &sparseFilters );

	uint64_t vertexOriIdxInProgress = 0;

	meshDataStruct* setMeshData = new meshDataStruct[availableConcurrentThreads];
	for( size_t t = 0; t < availableConcurrentThreads; t++ )
	{
		//cout << "[GigaMesh] Preparing data for thread " << t << endl;
		setMeshData[t].threadID               = t;
		setMeshData[t].meshToAnalyze          = &someMesh;
		setMeshData[t].radius                 = radius;
		setMeshData[t].xyzDim                 = xyzDim;
		setMeshData[t].multiscaleRadiiSize    = multiscaleRadiiSize;
		setMeshData[t].multiscaleRadii        = multiscaleRadii;
		setMeshData[t].sparseFilters          = &sparseFilters;
		setMeshData[t].vertexOriIdxInProgress = &vertexOriIdxInProgress;
		setMeshData[t].nrOfVerticesToProcess  = someMesh.getVertexNr();
		setMeshData[t].featureIndicies        = featureIndicies;
		setMeshData[t].patchNormal            = patchNormal;
		setMeshData[t].descriptVolume         = descriptVolume;
		setMeshData[t].descriptSurface        = descriptSurface;
	}

	int ctrIgnored{0};
	int ctrProcessed{0};

	if(availableConcurrentThreads < 2)
	{
		cout << "[GigaMesh] Thread 0 started" << endl;

		estFeatureVectors(setMeshData, 0, someMesh.getVertexNr());
	}

	else
	{

		const uint64_t offsetPerThread{someMesh.getVertexNr()/
			                                        availableConcurrentThreads};

		const uint64_t verticesPerThread{offsetPerThread};

		const uint64_t offsetThisThread{verticesPerThread*
			                                    (availableConcurrentThreads - 1)};

		const uint64_t verticesThisThread{offsetPerThread +
			                                    someMesh.getVertexNr() %
			                                    availableConcurrentThreads};

		std::vector<future<void>> threadFutureHandlesVector(availableConcurrentThreads - 1);

		for(unsigned int threadCount = 0;
		        threadCount < (availableConcurrentThreads - 1); threadCount++)
		{

			auto functionCall = std::bind(&estFeatureVectors,
			                                        &(setMeshData[threadCount]),
			                                        offsetPerThread*threadCount,
			                                        verticesPerThread);

			threadFutureHandlesVector.at(threadCount) =
			        std::async(std::launch::async, functionCall);
		}

		estFeatureVectors(&(setMeshData[availableConcurrentThreads - 1]),
		                                offsetThisThread, verticesThisThread);

		size_t threadCount{0};

		for(std::future<void>& threadFutureHandle : threadFutureHandlesVector)
		{
			threadFutureHandle.get();
			ctrIgnored   += setMeshData[threadCount].ctrIgnored;
			ctrProcessed += setMeshData[threadCount].ctrProcessed;
			++threadCount;
		}

	}

	delete[] setMeshData;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;
	fileStrOutMeta << "End date/time is:   " << asctime( timeinfo ); // no endl required as asctime will add a linebreak
	fileStrOutMeta << "Parallel processing took " << static_cast<int>( time( nullptr ) ) -  static_cast<int>( timeStampParallel ) << " seconds." << endl;

	cout << "[GigaMesh] Vertices processed: " << ctrProcessed << endl;
	cout << "[GigaMesh] Vertices ignored:   " << ctrIgnored << endl;
	cout << "[GigaMesh] Parallel processing took " << static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel )  << " seconds." << endl;
	cout << "[GigaMesh]               ... equals " << static_cast<int>( ctrProcessed ) /
	                ( static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel ) ) << " vertices/seconds." << endl;

	timeStampParallel = time( nullptr );

#ifndef GIGAMESH_PUBLIC_METHODS_ONLY
	// File for normal estimated:
	fstream filestrNormal;
	filestrNormal.open( fileNameOutPatchNormal.c_str(), fstream::out );
	filestrNormal << fixed << setprecision( 10 );
	for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
		if( featureIndicies[i] < 0 ) {
			//cout << "[GigaMesh] skip" << endl;
			continue;
		}
		filestrNormal << featureIndicies[i];
		// Normal - always three elements
		filestrNormal << " " << patchNormal[i*3];
		filestrNormal << " " << patchNormal[i*3+1];
		filestrNormal << " " << patchNormal[i*3+2];
		filestrNormal << endl;
	}
	filestrNormal.close();
	cout << "[GigaMesh] Patch normal stored in:                   " << fileNameOutPatchNormal << endl;
#endif

#ifdef GIGAMESH_PUBLIC_METHODS_ONLY
	delete descriptSurface;
	descriptSurface = NULL;
#endif

	// Feature vector file for BOTH descriptors (volume and surface)
	if( ( descriptSurface != NULL ) && ( descriptVolume != NULL ) ) {
		fstream filestrVS;
		filestrVS.open( fileNameOutVS.c_str(), fstream::out );
		filestrVS << fixed << setprecision( 10 );
		for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
			if( featureIndicies[i] < 0 ) {
				//cout << "skip" << endl;
				continue;
			}
			filestrVS << featureIndicies[i];
			// Scales - Volume:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrVS << " " << descriptVolume[i*multiscaleRadiiSize+j];
			}
			// Scales - Surface:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrVS << " " << descriptSurface[i*multiscaleRadiiSize+j];
			}
			filestrVS << endl;
		}
		filestrVS.close();
		cout << "[GigaMesh] Volume and surface descriptors stored in: " << fileNameOutVS << endl;
	}

	// Feature vector file for volume descriptor
	if( descriptVolume != NULL ) {
		fstream filestrVol;
		filestrVol.open( fileNameOutVol.c_str(), fstream::out );
		filestrVol << fixed << setprecision( 10 );
		for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
			if( featureIndicies[i] < 0 ) {
				//cout << "[GigaMesh] skip" << endl;
				continue;
			}
			Vertex* currVert = someMesh.getVertexPos( i );
			if( !currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize],
			                                 multiscaleRadiiSize ) ) {
				cerr << "[GigaMesh] Assignment of volume based feature vectors to vertices failed for Vertex No. " << i << "!" << endl;
			}
			filestrVol << featureIndicies[i];
			// Scales:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrVol << " " << descriptVolume[i*multiscaleRadiiSize+j];
			}
			filestrVol << endl;
		}
		filestrVol.close();
		cout << "[GigaMesh] Volume descriptors stored in:             " << fileNameOutVol << endl;

		// Apply feature vector metric.
		vector<double> referenceVector;
		double pNorm = 2.0;
		Mesh::eFuncFeatureVecPNormWeigth weigthChoosen = Mesh::FEATURE_VECTOR_PNORM_WEIGTH_CUBIC;
		referenceVector.resize( multiscaleRadiiSize, 0.0 );
		someMesh.funcVertFeatureVecPNorm( referenceVector, pNorm, weigthChoosen );

		// Save mesh having volumetric feature vectors.
		someMesh.writeFile( fileNameOut3D );
	}

	// Feature vector file for surface descriptor
	if( descriptSurface != NULL ) {
		fstream filestrSurf;
		filestrSurf.open( fileNameOutSurf.c_str(), fstream::out );
		filestrSurf << fixed << setprecision( 10 );
		for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
			if( featureIndicies[i] < 0 ) {
				//cout << "[GigaMesh] skip" << endl;
				continue;
			}
			//Vertex* currVert = someMesh.getVertexPos( i );
			//currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize], multiscaleRadiiSize );
			filestrSurf << featureIndicies[i];
			// Scales:
			for( uint j=0; j<multiscaleRadiiSize; j++ ) {
				filestrSurf << " " << descriptSurface[i*multiscaleRadiiSize+j];
			}
			filestrSurf << endl;
		}
		filestrSurf.close();
		cout << "[GigaMesh] Surface descriptors stored in:            " << fileNameOutSurf << endl;
	}
	cout << "[GigaMesh]                            " << (int)( time( nullptr ) - timeStampParallel ) << " seconds." << endl;

	delete[] featureIndicies;
	if( descriptVolume ) {
		delete[] descriptVolume;
	}
	if( descriptSurface  ) {
		delete[] descriptSurface;
	}
	delete[] multiscaleRadii;

	fileStrOutMeta.close();

	exit( EXIT_SUCCESS );
}
