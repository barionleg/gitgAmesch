
/*
Written by Martin Seiler
Part of the Master Thesis

Status: Experimental

Please look at mesh.cpp for larger and more complete disclaimer by original creators

*/

#ifndef WEDGEEXTRACTION_H
#define WEDGEEXTRACTION_H

#include <vector>
#include <list>

#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/face.h> //added 23.07. in experimental state to add extra vertices to mesh
#include <GigaMesh/mesh/mesh.h>


//helper methods exclusively called in the wedge extraction project


double wEComputeSquaredDistanceBetweenTwoVertices(Vertex* &vertexNo1, Vertex* &vertexNo2);

double wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(Vertex* &vertexNo1, Vertex* &vertexNo2);

double wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(Vertex* &vertexNo1, std::vector<double> &normalComponents);

void wERandomlyChooseVerticesFromVector(std::vector<Vertex*> &inputVector, std::vector<Vertex*> &outputVector, int howManyVerticesWanted);
/*
void wEFindMedianInListForGivenMean(std::list<Vertex*> &inputList, double &meanNormalX, double &meanNormalY, double &meanNormalZ, double &medianNormalX, double &medianNormalX, double &medianNormalX);
*/
void wEAssignNormalsToNearestMean(std::vector<int> &currentClustering, std::vector<Vertex*> &verticesWithCurrentLabel, std::vector<std::vector<double>> &normalMeanComponents);

void wEAssignNormalsToNearestMean(std::vector<int> &currentClustering, std::vector<Vertex*> &verticesWithCurrentLabel, std::vector<Vertex*> &threeRandomlyChosenVertices);

void wEComputeMeans(std::vector<int> &currentClustering, std::vector<Vertex*> &verticesWithCurrentLabel, std::vector<std::vector<double>> &normalMeanComponents);

double wEComputeShortestDistanceBetweenPointAndRay(Vertex* &arbPoint, Vertex* &point1RayStart, Vertex* &point2OnRay);
/*
double wEComputeShortestDistanceBetweenPointAndLine(Vertex* &arbPoint, Vertex* &point1OnLine, Vertex* &point2OnLine);
*/
/*
void wEGetBorderGroupFromVertexByFeatureVector(Vertex* vertexInQuestion,int &foundBordergroup);
*/
void wEGetBorderGroupFromVertexByFeatureVector(Vertex* &finalLineVertex1, Vertex* &finalLineVertex2, Vertex* &finalLineVertex3, Vertex* &finalTetraederVertexGroup12, Vertex* &finalTetraederVertexGroup23, Vertex* &finalTetraederVertexGroup31);

void wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(Vertex* &arbPoint, Vertex* &point1OnLine, Vertex* &point2OnLine, double &computedSquaredDistance);

double wEComputeTetraederHeight(std::vector<Vertex*> foundTetraeder);

bool wECheckTetraederHeight(std::vector<Vertex*> foundTetraeder, double &minimumHeight);

void wEWriteExtractedTetraedersIntoFile(std::vector<std::vector<Vertex*>> extractedTetraeders, std::string outputFileName);

void wETempNoteDownExtractedTetraeders(std::vector<std::vector<Vertex*>> &extractedTetraeders, int numberOfVertices, int numberOfFaces, std::vector<double> &RANSACQuality);

//copied from mesh.cpp, will be legacy soon
bool getSurroundingVerticesInOrder (std::list<Vertex*> &adjacentVertsInOrder, Vertex* &pi, bool printDebug);

void wEBuildVertexNeighbourLookUpStructure(std::vector<Vertex*> &mVertices, std::map<Vertex*,std::set<Vertex*>> &vertexNeighbourLookUp);


//methods called in mesh.cpp

bool experimentalSuppressNonMaxima(double &NMSDistance, std::vector<Vertex*> &mVertices);

bool experimentalComputeWatershed(double &watershedLimit, std::vector<Vertex*> &mVertices);

bool experimentalComputeClustering(int numberOfIterations, std::vector<Vertex*> &mVertices);

bool experimentalComputeRANSAC(	int numberOfIterations,
								std::string outputFileName,
								bool useNMSResultsForTetraederTop,
								double minimumTetraederHeight,
								bool extendMesh,
								bool addSeparationWall,
								bool visualizeRANSACQuality,
								bool visualizeTetraederHeight,
								std::vector<Vertex*> &mVertices,
								std::vector<Face*> &mFaces);

bool experimentalReorderFeatureVector(std::vector<Vertex*> &mVertices, std::vector<Face*> &mFaces, double deletableInput);

#endif // WEDGEEXTRACTION_H
