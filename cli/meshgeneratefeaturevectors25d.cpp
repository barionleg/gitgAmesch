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
		// output:
		int     ctrIgnored{0};
		int     ctrProcessed{0};
		// our most precious feature vectors (as array):
		double* patchNormal{nullptr};     //!< Normal used for orientation into 2.5D representation
		double* descriptVolume{nullptr};  //!< Volume descriptors
		double* descriptSurface{nullptr}; //!< Surface descriptors
		// and the voxel filter
		voxelFilter2DElements** sparseFilters{nullptr};
	};

//! Compute the Multi-Scale Integral Invariant feature vectors
void estFeatureVectors(
                meshDataStruct* meshData,
                const size_t threadOffset,
                const size_t threadVertexCount
) {

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
		double* tDescriptVolume     = meshData->descriptVolume;  //!< Volume descriptors
		double* tDescriptSurface    = meshData->descriptSurface; //!< Surface descriptors
		double* tSurfacePatchNormal = meshData->patchNormal;     //!< Surface patch normal

		// setup memory for rastered surface:
		const uint    rasterSize = meshData->xyzDim * meshData->xyzDim;
		double* rasterArray = new double[rasterSize];

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
			if( tSurfacePatchNormal ) {
				vector<Face*>::iterator itFace;
				for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
					(*itFace)->addNormalTo( &(tSurfacePatchNormal[vertexOriIdxInProgress*3]) );
				}
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
				double time_remaining =   (time_elapsed / percentDone) - time_elapsed;
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
} // END of estFeatureVectors

//! Process one file with the given paramters.
bool generateFeatureVectors(
                const std::filesystem::path&   fileNameIn,
                const std::filesystem::path&   rOptFileSuffix,
                double       radius,
                unsigned int xyzDim,
                unsigned int radiiCount,
                bool         replaceFiles,
                bool                           rAreaOnly,
                bool                           rConcatResults,
                const char*                    rHostname,
                const char*                    rUsername
) {
	// Check existance of the input file:
	if( !std::filesystem::exists(fileNameIn) ) {
		std::cerr << "[GigaMesh] ERROR: File '" << fileNameIn << "' not found!" << std::endl;
		return( false );
	}

	// Check extension:
	if( !fileNameIn.has_extension() ) {
		std::cerr << "[GigaMesh] ERROR: No extension/type for the given input file '" << fileNameIn << "' specified!" << std::endl;
		return( false );
	}

	// Fileprefix for output
	std::filesystem::path fileNameOut( fileNameIn );
	fileNameOut.replace_extension( "" );

	// Prepare suffix for the output file
	char tmpBuffer[512];
	sprintf( tmpBuffer, "_r%0.2f_n%i_v%i", radius, radiiCount, xyzDim );
	fileNameOut += rOptFileSuffix;
	fileNameOut += string( tmpBuffer );

	// Check: Output file for normal used to rotate the local patch
	std::filesystem::path fileNameOutPatchNormal( fileNameOut );
	fileNameOutPatchNormal += ".normal.mat";
	if( std::filesystem::exists(fileNameOutPatchNormal) ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutPatchNormal << "' already exists!" << endl;
			return( false );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutPatchNormal << "' will be replaced!" << endl;
	}

	// Output file for volume descriptor (1st integral invariant)
	std::filesystem::path fileNameOutVol;
	if( !rAreaOnly ) {
		// Check: Output file for volume descriptor
		fileNameOutVol = fileNameOut;
		fileNameOutVol += ".volume.mat";
		if( std::filesystem::exists( fileNameOutVol ) ) {
			if( !replaceFiles ) {
				cerr << "[GigaMesh] File '" << fileNameOutVol << "' already exists!" << endl;
				return( false );
			}
			cerr << "[GigaMesh] Warning: File '" << fileNameOutVol << "' will be replaced!" << endl;
		}
	}

	// Output for the concatenated results (1st and 2nd integral invariant)
	std::filesystem::path fileNameOutVS;
	if( !rAreaOnly & rConcatResults ) {
		// Check: Output file for volume AND surface descriptor
		fileNameOutVS = fileNameOut;
		fileNameOutVS += ".vs.mat";
		if( std::filesystem::exists(fileNameOutVS)) {
			if( !replaceFiles ) {
				cerr << "[GigaMesh] File '" << fileNameOutVS << "' already exists!" << endl;
				return( false );
			}
			cerr << "[GigaMesh] Warning: File '" << fileNameOutVS << "' will be replaced!" << endl;
		}
	}

	// Output file for surface descriptor (2nd integral invariant)
	std::filesystem::path fileNameOutSurf( fileNameOut );
	fileNameOutSurf += ".surface.mat";
	if( std::filesystem::exists( fileNameOutSurf ) ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutSurf << "' already exists!" << endl;
			return( false );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutSurf << "' will be replaced!" << endl;
	}

	// Output file for the technical meta-data
	std::filesystem::path fileNameOutMeta( fileNameOut );
	fileNameOutMeta += ".info.txt";
	if( std::filesystem::exists( fileNameOutMeta ) ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOutMeta << "' already exists!" << endl;
			return( false );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOutMeta << "' will be replaced!" << endl;
	}

	// Output file for 3D data including the (1st) volumetric feature vectors.
	std::filesystem::path fileNameOut3D( fileNameOut );
	fileNameOut3D += ".ply";
	if( std::filesystem::exists( fileNameOut3D )  ) {
		if( !replaceFiles ) {
			cerr << "[GigaMesh] File '" << fileNameOut3D << "' already exists!" << endl;
			return( false );
		}
		cerr << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!" << endl;
	}

	// All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
	fstream fileStrOutMeta;
	fileStrOutMeta.open( fileNameOutMeta, fstream::out );
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
	std::cout << setprecision( 2 ) << std::fixed;
	fileStrOutMeta << setprecision( 2 ) << std::fixed;

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

	// Info about files
	std::cout << "[GigaMesh] File to write technical Meta-Data:   " << fileNameOutMeta << std::endl;
	if( !fileNameOutVol.empty() ) {
		std::cout << "[GigaMesh] File to write Volume (1st MSII):     " << fileNameOutVol << std::endl;
	}
	if( !fileNameOutSurf.empty() ) {
		std::cout << "[GigaMesh] File to write Surface (2nd MSII):    " << fileNameOutSurf << std::endl;
	}
	if( !fileNameOutPatchNormal.empty() ) {
		std::cout << "[GigaMesh] File to write Surface patch normal:  " << fileNameOutPatchNormal << std::endl;
	}
	if( !fileNameOutVS.empty() ) {
		std::cout << "[GigaMesh] File to write concatenated V+S:      " << fileNameOutVS << std::endl;
	}

	// Prepare data structures
	//--------------------------------------------------------------------------
	bool readSucess;
	Mesh someMesh( fileNameIn, readSucess );
	if( !readSucess ) {
		cerr << "[GigaMesh] Error: Could not open file '" << fileNameIn << "'!" << endl;
		return( false );
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
	// Write technical meta-data to file
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
	fileStrOutMeta << "Hostname:           " << rHostname << std::endl;
	fileStrOutMeta << "Username:           " << rUsername << std::endl;

	double* descriptVolume{nullptr};
	if( !rAreaOnly ) {
		descriptVolume  = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	}

	double* descriptSurface{nullptr};
	descriptSurface = new double[someMesh.getVertexNr()*multiscaleRadiiSize];

	double* patchNormal{nullptr};
	patchNormal     = new double[someMesh.getVertexNr()*3];

	// Initialize array, when required=allocated.
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
	if( patchNormal != NULL ) {
		for( uint64_t i=0; i<someMesh.getVertexNr()*3; i++ ) {
			patchNormal[i] = 0.0;
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
		setMeshData[t].patchNormal            = patchNormal;
		setMeshData[t].descriptVolume         = descriptVolume;
		setMeshData[t].descriptSurface        = descriptSurface;
	}

	int ctrIgnored{0};
	int ctrProcessed{0};

	if(availableConcurrentThreads < 2)
	{
		std::cout << "[GigaMesh] Thread 0 started" << std::endl;
		estFeatureVectors(setMeshData, 0, someMesh.getVertexNr());
	} else {
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
	std::cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;
	fileStrOutMeta << "End date/time is:   " << asctime( timeinfo ); // no endl required as asctime will add a linebreak
	fileStrOutMeta << "Parallel processing took " << static_cast<int>( time( nullptr ) ) -  static_cast<int>( timeStampParallel ) << " seconds." << std::endl;

	std::cout << "[GigaMesh] Vertices processed: " << ctrProcessed << std::endl;
	std::cout << "[GigaMesh] Vertices ignored:   " << ctrIgnored << std::endl;
	std::cout << "[GigaMesh] Parallel processing took " << static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel )  << " seconds." << std::endl;
	std::cout << "[GigaMesh]               ... equals " << static_cast<int>( ctrProcessed ) /
	                ( static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel ) + 0.1 ) << " vertices/seconds." << std::endl; // add 0.1 to avoid division by zero for small meshes.

	timeStampParallel = time( nullptr );

	bool retVal(true);

	// Feature vector file for volume descriptor (1st integral invariant)
	if( (!fileNameOutVol.empty()) && ( descriptVolume != NULL ) ) {
		fstream filestrVol;
		filestrVol.open( fileNameOutVol, fstream::out );
		if( !filestrVol.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutVol << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrVol << fixed << setprecision( 10 );
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				Vertex* currVert = someMesh.getVertexPos( i );
				if( !currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize],
				                                 multiscaleRadiiSize ) ) {
					std::cerr << "[GigaMesh] Assignment of volume based feature vectors to vertices failed for Vertex No. " << i << "!" << std::endl;
				}
				// Index:
				filestrVol << i;
				// Scales or elements of the feature vector:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrVol << " " << descriptVolume[i*multiscaleRadiiSize+j];
				}
				filestrVol << std::endl;
			}
			filestrVol.close();
			std::cout << "[GigaMesh] Volume descriptors stored in:             " << fileNameOutVol << std::endl;
		}
	}

	// Feature vector file for surface descriptor (2nd integral invariant)
	if( (!fileNameOutSurf.empty()) && ( descriptSurface != NULL )) {
		fstream filestrSurf;
		filestrSurf.open( fileNameOutSurf, fstream::out );
		if( !filestrSurf.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutSurf << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrSurf << fixed << setprecision( 10 );
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				//Vertex* currVert = someMesh.getVertexPos( i );
				//currVert->assignFeatureVec( &descriptSurface[i*multiscaleRadiiSize], multiscaleRadiiSize );
				filestrSurf << i;
				// Scales:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrSurf << " " << descriptSurface[i*multiscaleRadiiSize+j];
				}
				filestrSurf << endl;
			}
			filestrSurf.close();
			std::cout << "[GigaMesh] Surface descriptors stored in:            " << fileNameOutSurf << std::endl;
		}
	}

	// File for normal estimated as byproduct of the 2nd integral invariant:
	if( (!fileNameOutPatchNormal.empty()) && ( patchNormal != NULL ) ) {
		fstream filestrNormal;
		filestrNormal.open( fileNameOutPatchNormal, fstream::out );
		if( !filestrNormal.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutPatchNormal << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrNormal << fixed << setprecision( 10 );
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				// Index of the vertex
				filestrNormal << i;
				// Normal - always three elements
				filestrNormal << " " << patchNormal[i*3];
				filestrNormal << " " << patchNormal[i*3+1];
				filestrNormal << " " << patchNormal[i*3+2];
				filestrNormal << std::endl;
			}
			filestrNormal.close();
			std::cout << "[GigaMesh] Patch normal stored in:                   " << fileNameOutPatchNormal << std::endl;
		}
	}

	// Feature vector file for BOTH descriptors (volume and surface)
	if( (!fileNameOutVS.empty()) && ( descriptSurface != NULL ) && ( descriptVolume != NULL ) ) {
		fstream filestrVS;
		filestrVS.open( fileNameOutVS, fstream::out );
		if( !filestrVS.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutVS << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrVS << fixed << setprecision( 10 );
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				filestrVS << i;
				// Scales - Volume:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrVS << " " << descriptVolume[i*multiscaleRadiiSize+j];
				}
				// Scales - Surface:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrVS << " " << descriptSurface[i*multiscaleRadiiSize+j];
				}
				filestrVS << std::endl;
			}
			filestrVS.close();
			std::cout << "[GigaMesh] Volume and surface descriptors stored in: " << fileNameOutVS << std::endl;
		}
	}

	// PLY file INCLUDING the feature vector for volume descriptor (1st integral invariant)
	// as well as a newly computed function value.
	if( (!fileNameOut3D.empty()) && ( descriptVolume != NULL ) ) {
		// Apply feature vector metric:
		//--------------------------------------
		//vector<double> referenceVector;
		//double pNorm = 2.0;
		//Mesh::eFuncFeatureVecPNormWeigth weigthChoosen = Mesh::FEATURE_VECTOR_PNORM_WEIGTH_CUBIC;
		//referenceVector.resize( multiscaleRadiiSize, 0.0 );
		//someMesh.funcVertFeatureVecPNorm( referenceVector, pNorm, weigthChoosen );

		// Apply (simple) feature vector metric:
		//--------------------------------------
		someMesh.funcVertFeatureVecMax();
		// Save mesh having volumetric feature vectors.
		if( !someMesh.writeFile( fileNameOut3D ) ) {
			retVal = false;
		} else {
			std::cout << "[GigaMesh] PLY with volume descriptors stored in:    " << fileNameOut3D << std::endl;
		}
	}

	// Done
	std::cout << "[GigaMesh] Writing the files took " << static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel ) << " seconds." << std::endl;

	if( descriptVolume ) {
		delete[] descriptVolume;
	}
	if( descriptSurface  ) {
		delete[] descriptSurface;
	}
	delete[] multiscaleRadii;

	fileStrOutMeta.close();
	return( retVal );
}

//! Show software version.
void printVersion() {
	std::cout << "GigaMesh Software Framework FEATUREVECTORS 3D-data " << VERSION_PACKAGE << std::endl;
#ifdef THREADS
	std::cout << "Multi-threading with " << std::thread::hardware_concurrency() * 2 << " (dynamic) threads." << std::endl;
#else
	std::cout << "Single-threading. " << std::endl;
#endif
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName )
{
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
	std::cout << "GigaMesh Software Framework FEATUREVECTORS (1st and 2nd or V and P)" << std::endl << std::endl;
	std::cout << "Computes the first and second integral invariants for the given meshes on multiple scales\n"
	             "shortly known as MSII filtering. The first integral invariant computes the Volume, while \n"
	             "the second computes the Patch surface integral invariant." << std::endl;
	std::cout << "Note: for the 3rd and 4th integral invariant see 'gigamesh-featurevectors-sl'." << std::endl;
	std::cout << std::endl;
	std::cout << "The output file(s) will be namend the same as the input file " << std::endl;
	std::cout << "always using the following suffix:" << std::endl;
	std::cout << std::endl;
	std::cout << "  _r<number> ... radius of the largest sphere/scale" << std::endl;
	std::cout << "  _n<number> ... 2^<number> of scales" << std::endl;
	std::cout << "  _v<number> ... discretization for the largest scale. Large numbers are more precise\n"
	             "                 resulting in slower computation" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help                              Displays this help." << std::endl;
	std::cout << "  -v, --version                           Displays version information." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for output files:" << std::endl;
	std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is not done by default" << std::endl;
	std::cout << "                                          to prevent accidental data loss. If this option is not set" << std::endl;
	std::cout << "                                          and the output file exists, the file will be skipped." << std::endl;
	std::cout << "  -s, --output-suffix <string>            Write the converted file using the given <string> as suffix for its name." << std::endl;
	std::cout << "                                          This options has no effect on the suffix created for the parameters." << std::endl;
	std::cout << "    , --concat-results                    Concatenate 1st and 2nd feature vector and store it in one file." << std::endl;
	std::cout << "                                          The file has the extenstion .vs.mat." << std::endl;
	std::cout << "                                          Has no effect, when only one integral invariant is computed." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for MSII filtering:" << std::endl;
	std::cout << "  -r, --radius SIZE                       Radius of the largest sphere/scale. Default is 1.0 (mm, unit assumed!)" << std::endl;
	std::cout << "                                          Recommended: should be equal or larger than the largest feature to be detected." << std::endl;
	std::cout << "  -n, --numScales SIZE                    Power of two for the number of scales. Default is 4 i.e. 16 scales." << std::endl;
	std::cout << "                                          Recommended: The default works well for most applications." << std::endl;
	std::cout << "  -l, --voxelSize SIZE                    Discretization for the (1st) volume integral invariant. Default is 256." << std::endl;
	std::cout << "                                          Recommended: should be a power of two. Values of 512 and 1024 can increase the results" << std::endl;
	std::cout << "                                          slightly at the cost of extra compute time." << std::endl;
	std::cout << "  -2, --areaintegralOnly                  Compute only the (2nd) patch area integral invariant." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for testing and debugging:" << std::endl;
	std::cout << "    , --log-level [0-4]                   Sets the log level of this application.\n"
	             "                                          Higher numbers increases verbosity.\n"
	             "                                          (Default: 1)" << std::endl;
	//std::cout << "" << std::endl;
}

//! Main routine for generating an array of feature vectors
//!
//! Remark: prefer MeshSeed over Mesh as it is faster by a factor of 3+
//==========================================================================
int main( int argc, char *argv[] ) {

	LOG::initLogging();

	// Default arameters
	std::filesystem::path optFileSuffix;
	double       radius{_DEFAULT_FEATUREGEN_RADIUS_};
	unsigned int xyzDim{_DEFAULT_FEATUREGEN_XYZDIM_};
	unsigned int radiiCount{_DEFAULT_FEATUREGEN_RADIICOUNT_};
	bool         replaceFiles{false};
	bool         areaOnly{false};
	bool         concatResults{false};

	static struct option longOptions[] = {
		{ "radius"            , required_argument, nullptr, 'r' },
		{ "voxelSize"         , required_argument, nullptr, 'l' },
		{ "numScales"         , required_argument, nullptr, 'n' },
		{ "overwrite-existing", no_argument      , nullptr, 'k' },
		{ "areaintegralOnly"  , no_argument      , nullptr, '2' },
		{ "output-suffix"     , required_argument, nullptr, 's' },
		{ "concat-results"    , no_argument      , nullptr,  0  },
		{ "version"           , no_argument,       nullptr, 'v' },
		{ "help"              , no_argument      , nullptr, 'h' },
		{ "log-level"         , required_argument, nullptr,  0  },
		{ nullptr, 0, nullptr, 0 }
	};

	// PARSE command line options
	//--------------------------------------------------------------------------
	opterr = 0;
	int c{0};
	int optionIndex{0};
	int tmpInt{0};
	bool radiusSet{false};
	while( ( c = getopt_long_only( argc, argv, "r:l:n:k2s:vh",
	                               longOptions, &optionIndex) ) != -1 ) {
		switch( c ) {
			//! Option r: absolut radius (in units, default: 1.0)
			case 'r':
				radius = atof( optarg );
				radiusSet = true;
				break;
			//! Option v: edge length of the voxel cube (in voxels, default: 256)
			case 'l':
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
			case '2':
				cout << "[GigaMesh] Warning: Only area integrals will be computed!" << endl;
				areaOnly = true;
				break;
			//! Option s: output file suffix
			case 's':
				optFileSuffix = std::string( optarg );
				break;

			case 'h':
				printHelp( argv[0] );
				std::exit( EXIT_SUCCESS );
				break;
			case 'v':
				printVersion();
				std::exit( EXIT_SUCCESS );
				break;
			case '?':
				std::cerr << "[GigaMesh] ERROR: Unknown option!" << std::endl;
				break;
			// Non-short options:
			case 0:
				if( std::string(longOptions[optionIndex].name) == "log-level" ) {
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
				if( std::string(longOptions[optionIndex].name) == "concat-results" ) {
					concatResults = true;
				}
				break;
			default:
				std::cerr << "[GigaMesh] Error: Unknown option '" << c << "'!" << std::endl;
				std::exit( EXIT_FAILURE );
		}
	}
	// Check argument ranges
	if( radius <= 0.0 ) {
		std::cerr << "[GigaMesh] Error: negative or zero radius given: " << radius << " (option -r)!" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	if( !radiusSet ) {
		std::cout << "[GigaMesh] Warning: default radius is used (option -r missing)!" << std::endl;
	}

	// SHOW Build information
	printBuildInfo();

	// Fetch username and host for the technical meta-data
#ifdef WIN32
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char hostname[256] = {0};
	auto error = gethostname(hostname, 256);

	if(error != 0)
	{
		std::cerr << "[GigaMesh] ERROR: Could not get hostname!" << std::endl;
	}
	char username[UNLEN- 1] = {0};
	DWORD len = UNLEN - 1;
	if(!GetUserNameA(username,&len))
	{
		std::cerr << "[GigaMesh] ERROR: Could not get username!" << std::endl;
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
		std::cerr << "[GigaMesh] ERROR: Could not get username!" << std::endl;
	}
#endif

	unsigned long filesProcessed = 0UL;
	unsigned long filesFailed    = 0UL;

	for(auto nonOptionArgumentIndex = optind; nonOptionArgumentIndex < argc; ++nonOptionArgumentIndex)
	{
		std::filesystem::path nonOptionArgumentPath(argv[nonOptionArgumentIndex]);

		if(std::filesystem::exists(nonOptionArgumentPath))
		{
			std::wcout << L"[GigaMesh] Processing file " << nonOptionArgumentPath.wstring() << L"..." << std::endl;

			if( !generateFeatureVectors( nonOptionArgumentPath,
			                             optFileSuffix,
			                             radius,
			                             xyzDim,
			                             radiiCount,
			                             replaceFiles,
			                             areaOnly,
			                             concatResults,
			                             hostname, username
			                           ) )
			{
				LOG::error() << "[GigaMesh] ERROR: generate featurevectors failed for: " << nonOptionArgumentPath.string() << " !\n";
				filesFailed++;
			}
			++filesProcessed;
		}
	}

	// No file was processed:
	if( filesProcessed == 0 ) {
		std::cout << "[GigaMesh] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		std::cerr << "[GigaMesh] ERROR: no filename for input given!" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Some files were not processed:
	if( filesFailed > 0 ) {
		std::cout << "[GigaMesh] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		std::cerr << "[GigaMesh] ERROR: " << filesFailed << " of " << filesProcessed << " files could not be processed!" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Everything fine:
	std::cout << "[GigaMesh] ========================================================" << std::endl;
	std::cout << "[GigaMesh] successfully processed " << filesProcessed << " file(s)." << std::endl;
	std::exit( EXIT_SUCCESS );
}
