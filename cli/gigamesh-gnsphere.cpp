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

#include <stdio.h>
#include <stdlib.h> // calloc
#include <string>
#ifdef _MSC_VER	//windows version for hostname and login
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
                const filesystem::path&   rFileName,
                const filesystem::path&   rOutputPath,
                int& rSubdivisionLevel,
                const bool      rFaceNormals
) {
        if( rFileName.extension().wstring().size() != 4 ) {
                cerr << "[GigaMesh] ERROR: File extension '" << rFileName.extension().string() << "' is faulty!" << endl;
                return( false );
        }

        // Prepare data structures
        //--------------------------------------------------------------------------
        bool readSucess;
        Mesh someMesh( rFileName, readSucess );
        if( !readSucess ) {
                cerr << "[GigaMesh] Error: Could not open file '" << rFileName << "'!" << endl;
                return( false );
        }

        //prepare normal data
        bool sphereCoordinates = true;

        std::list<sVertexProperties> vertexProps;

        auto fAddNormal = [&vertexProps](const Vector3D& normal) -> void {
            if( std::isnan(normal.getX()) || std::isnan(normal.getY()) || std::isnan(normal.getZ()) )
            {
                return;
            }
            sVertexProperties prop;
            prop.mNormalX = normal.getX();
            prop.mNormalY = normal.getY();
            prop.mNormalZ = normal.getZ();

            vertexProps.emplace_back(prop);
        };
        if(rFaceNormals)
        {
            auto faceCount = someMesh.getFaceNr();

            for( uint64_t faceIdx = 0; faceIdx < faceCount; ++faceIdx)
            {
                Face* currFace = someMesh.getFacePos( faceIdx );
                auto normal = currFace->getNormal(false);
                fAddNormal(normal);
            }
        }
        else
        {
            auto vertCount = someMesh.getVertexNr();

            for( uint64_t vertIdx=0; vertIdx<vertCount; vertIdx++ ) {
                Vertex* currVertex = someMesh.getVertexPos( vertIdx );
                auto normal = currVertex->getNormal(false);
                fAddNormal(normal);
            }
        }
        // Write data to file

        time_t     rawtime;
        struct tm* timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );
        cout << "[GigaMesh] Start date/time is: " << asctime( timeinfo );// << endl;
        someMesh.writeIcoNormalSphereData(rOutputPath, vertexProps , rSubdivisionLevel, sphereCoordinates);
        cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;

        return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
        std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
        std::cout << "GigaMesh Software Framework export gaussian normal sphere data " << std::endl << std::endl;
        std::cout << "Exports the gaussian normal sphere data of the given mesh" << std::endl;
        std::cout << std::endl << std::endl;
        std::cout << "Options:" << endl;
        std::cout << "  -h, --help                              Displays this help." << std::endl;
        std::cout << "  -v, --version                           Displays version information." << std::endl << std::endl;
        std::cout << "  -s, --subdivision-level                 subdivision-level of the export" << std::endl;
        std::cout << "  -f, --face-normals                      Export the face normals. Default: Vertex Normals" << std::endl;
        std::cout << "  -o, --output-path <string>              Path to save the export. Default: gaussian_normals.csv" << std::endl;
}

//! Main routine for loading a (binary) PLY and store it as ASCII without the extra data supplied by GigaMesh
//==============================================================================================================================================================
int main( int argc, char *argv[] ) {

        LOG::initLogging();

        // Default string parameter
        std::filesystem::path optOutputPath;

        // Default flags
        bool optFaceNormals = false;
        int subdivisionLevel = 6;
        // PARSE command line options
        //--------------------------------------------------------------------------
        // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
        static struct option longOptions[] = {
                { "output-path",                required_argument, nullptr, 'o' },
                { "subdivision-level",                     required_argument, nullptr, 's'},
                { "face-normals",           no_argument,       nullptr, 'f' },
                { "version",                      no_argument,       nullptr, 'v' },
                { "help",                         no_argument,       nullptr, 'h' },
                { nullptr, 0, nullptr, 0 }
        };

        int character = 0;
        int optionIndex = 0;

        while( ( character = getopt_long_only( argc, argv, ":s:bnkvh",
                 longOptions, &optionIndex ) ) != -1 ) {
                switch(character) {

                        case 'o': // output file
                                optOutputPath = std::string( optarg );
                                break;

                        case 'f': // export face normals
                                optFaceNormals = true;
                                break;

                        case 's': // subdivision level
                                subdivisionLevel = stoi(std::string( optarg ));
                                break;

                        case 'v':
                                std::cout << "GigaMesh Software Framework GNSPHERE Stanford Polygon Files (PLYs) " << VERSION_PACKAGE << endl;
                                std::cout << "Multi-threading with " << std::thread::hardware_concurrency() - 1 << " threads." << endl;
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
        if( optOutputPath.empty() ) {
                optOutputPath = "gaussian_normals.csv";
        }

        // SHOW Build information
        printBuildInfo();

        // Process given files
        unsigned long filesProcessed = 0;
        for( int nonOptionArgumentCount = optind;
             nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

                std::filesystem::path nonOptionArgumentString ( argv[nonOptionArgumentCount] );

                if( !nonOptionArgumentString.empty() ) {
                        std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

                        if( !convertMeshData( nonOptionArgumentString, optOutputPath, subdivisionLevel,
                                              optFaceNormals) ) {
                                std::cerr << "[GigaMesh] ERROR: export Normalsphere failed!" << std::endl;
                                std::exit( EXIT_FAILURE );
                        }
                        filesProcessed++;
                }
        }

        std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
        exit( EXIT_SUCCESS );
}
