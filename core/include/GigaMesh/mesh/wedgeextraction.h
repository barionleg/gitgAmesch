
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
/*
void wEGetBorderGroupFromVertexByFeatureVector(Vertex* vertexInQuestion,int &foundBordergroup);
*/
void wEGetBorderGroupFromVertexByFeatureVector(Vertex* &finalLineVertex1, Vertex* &finalLineVertex2, Vertex* &finalLineVertex3, Vertex* &finalTetraederVertexGroup12, Vertex* &finalTetraederVertexGroup23, Vertex* &finalTetraederVertexGroup31);

void wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(Vertex* &arbPoint, Vertex* &point1OnLine, Vertex* &point2OnLine, double &computedSquaredDistance);

/*
void writeWedgeToPlyOrObj();
*/

//copied from mesh.cpp, will be legacy soon
bool getSurroundingVerticesInOrder (std::list<Vertex*> &adjacentVertsInOrder, Vertex* &pi, bool printDebug);

//methods called in mesh.cpp

bool experimentalSuppressNonMaxima(double &NMSDistance, std::vector<Vertex*> &mVertices);

bool experimentalComputeWatershed(double deletableInput, std::vector<Vertex*> &mVertices);

bool experimentalComputeClustering(int numberOfIterations, std::vector<Vertex*> &mVertices);

bool experimentalComputeRANSAC(int numberOfIterations, std::vector<Vertex*> &mVertices);

bool experimentalReorderFeatureVector(std::vector<Vertex*> &mVertices);

#endif // WEDGEEXTRACTION_H
