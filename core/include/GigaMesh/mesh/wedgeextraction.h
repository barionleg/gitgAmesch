
/*
Written by Martin Seiler
Part of the Master Thesis

Status: Experimental

Please look at mesh.cpp for larger and more complete disclaimer by original creators

*/

#ifndef WEDGEEXTRACTION_H
#define WEDGEEXTRACTION_H

//#include <iostream>


//helper methods exclusively called in the wedge extraction project


double squaredDistanceBetweenTwoVertices(Vertex* &vertexNo1, Vertex* &vertexNo2);

double squaredDistanceBetweenTwoNormalsTreatedAsVertices(Vertex* &vertexNo1, Vertex* &vertexNo2);

void wERandomlyChooseVerticesFromVector(vector<Vertex*> &inputVector, vector<Vertex*> &outputVector, int howManyVerticesWanted);

void wEFindMedianInListForGivenMean(list<Vertex*> &inputList, double &meanNormalX, double &meanNormalY, double &meanNormalZ, double &medianNormalX, double &medianNormalX, double &medianNormalX);

void wEAssignNormalsToNearestMean(vector<double> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<Vertex*> &threeVerticesWhoseNormalsAreUsed);

void wEComputeMean(vector<double> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<Vertex*> &threeVerticesWhoseNormalsAreUsed);

void wEGetBorderGroupFromVertexByFeatureVector(Vertex* vertexInQuestion,int &foundBordergroup);

void wEDistanceFromTetraederTopToProjectedPointOnLine(Vertex* &arbPoint, Vertex* &point1OnLine, Vertex* &point2OnLine, double &computedDistance);

//methods called in mesh.cpp

bool experimentalNonMaximumSuppression(double &NMSDistance, vector<Vertex*> &mVertices, uint64_t &numberOfVertices);

bool experimentalWaterShed(vector<Vertex*> &mVertices, uint64_t &numberOfVertices);

bool experimentalClustering(int numberOfIterations, vector<Vertex*> &mVertices);

bool experimentalRANSAC(int numberOfIterations, vector<Vertex*> &mVertices);

bool experimentalFeatureVectorReordering(vector<Vertex*> &mVertices);

#endif // WEDGEEXTRACTION_H
