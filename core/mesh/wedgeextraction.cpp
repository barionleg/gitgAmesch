
/*
Written by Martin Seiler
Part of the Master Thesis

Status: Experimental

Please look at mesh.cpp for larger and more complete disclaimer by original creators

*/

#include<iostream>
#include<fstream> //to write to a file
#include<vector>
#include<set>
#include<list>
#include<iterator>
#include<queue>
#include<map>

#include<algorithm>
#include<random>
#include<chrono>
#include<math.h> //for sqrt, can hopefully be omitted

#include<limits> //used for infinity placeholders in RANSAC

#include <GigaMesh/mesh/wedgeextraction.h>

#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/face.h> 			//for mesh extension, experimental state
#include <GigaMesh/mesh/vertexofface.h> 	//for mesh extension, experimental state
#include <GigaMesh/mesh/mesh.h>


using namespace std;

//helper methods exclusively called in the wedge extraction project
//they all begin with wE

//double temporaryRANSACParameter = 0.1;
double temporaryNMSParameter = 0.1;
double temporaryWaterParameter = 0.1;

//! Distance calculation for vertices omitting square root for speed up
double wEComputeSquaredDistanceBetweenTwoVertices(Vertex* &vertexNo1, Vertex* &vertexNo2){
/*
	return(	(vertexNo1->getX() - vertexNo2->getX()) * (vertexNo1->getX() - vertexNo2->getX())	+
			(vertexNo1->getY() - vertexNo2->getY()) * (vertexNo1->getY() - vertexNo2->getY())	+
			(vertexNo1->getZ() - vertexNo2->getZ()) * (vertexNo1->getZ() - vertexNo2->getZ())	);
*/

	return(	pow((vertexNo1->getX() - vertexNo2->getX()), 2.0)+
			pow((vertexNo1->getY() - vertexNo2->getY()), 2.0)+
			pow((vertexNo1->getZ() - vertexNo2->getZ()), 2.0)	);

}


//! Distance calculation for normals interpreted as vertices omitting square root for speed up
double wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(Vertex* &vertexNo1, Vertex* &vertexNo2){
/*
	return(	(vertexNo1->getX() - vertexNo2->getX()) * (vertexNo1->getX() - vertexNo2->getX())	+
			(vertexNo1->getY() - vertexNo2->getY()) * (vertexNo1->getY() - vertexNo2->getY())	+
			(vertexNo1->getZ() - vertexNo2->getZ()) * (vertexNo1->getZ() - vertexNo2->getZ())	);
*/

	return(	pow((vertexNo1->getNormalX() - vertexNo2->getNormalX()), 2.0)+
			pow((vertexNo1->getNormalY() - vertexNo2->getNormalY()), 2.0)+
			pow((vertexNo1->getNormalZ() - vertexNo2->getNormalZ()), 2.0)	);

}


//! Distance calculation for normals interpreted as vertices omitting square root for speed up
//! Input is a vertex and a normal given as a vector of doubles
double wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(Vertex* &vertexNo1, vector<double> &normalComponents){

	return(	pow((vertexNo1->getNormalX() - normalComponents[0]), 2.0)+
			pow((vertexNo1->getNormalY() - normalComponents[1]), 2.0)+
			pow((vertexNo1->getNormalZ() - normalComponents[2]), 2.0)	);

}


//! Random Selection of a specified number of Vertices from a vector
//! Shuffles the input vector for the random selection, there may be quicker ways
void wERandomlyChooseVerticesFromVector(vector<Vertex*> &inputVector, vector<Vertex*> &outputVector, int howManyVerticesWanted){

	unsigned timeBasedSeed = chrono::system_clock::now().time_since_epoch().count();

	shuffle(inputVector.begin(), inputVector.end(), default_random_engine(timeBasedSeed));

	//will be rewritten to be less absolute soon...
	if(howManyVerticesWanted == 3){
		outputVector.push_back(inputVector[0]);
		outputVector.push_back(inputVector[1]);
		outputVector.push_back(inputVector[2]);
	}

	if(howManyVerticesWanted == 4){
		outputVector.push_back(inputVector[0]);
		outputVector.push_back(inputVector[1]);
		outputVector.push_back(inputVector[2]);
		outputVector.push_back(inputVector[3]);
	}

}

/*
//! currently unused, no necessity because mean is used instead of median
void wEFindMedianInListForGivenMean(list<Vertex*> &inputList, double &meanNormalX, double &meanNormalY, double &meanNormalZ, double &medianNormalX, double &medianNormalY, double &medianNormalZ){

	//check back with supervisor

}
*/

//! Notes down to which mean a Normal is nearest in a euclidian sense. This means, normals are interpreted as points
void wEAssignNormalsToNearestMean(vector<int> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<vector<double>> &normalMeanComponents){

	for(int i=0; i<verticesWithCurrentLabel.size(); i++){

		double distanceToNormal1 = wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(verticesWithCurrentLabel[i], normalMeanComponents[0]);
		double distanceToNormal2 = wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(verticesWithCurrentLabel[i], normalMeanComponents[1]);
		double distanceToNormal3 = wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(verticesWithCurrentLabel[i], normalMeanComponents[2]);

		if((distanceToNormal1 <= distanceToNormal2) && (distanceToNormal1 <= distanceToNormal3)) {
			//normal 1 is nearest
			currentClustering[i] = 1;
		} else if ((distanceToNormal2 <= distanceToNormal1) && (distanceToNormal2 <= distanceToNormal3)){
			//normal 2 is nearest
			currentClustering[i] = 2;
		} else if ((distanceToNormal3 <= distanceToNormal1) && (distanceToNormal3 <= distanceToNormal2)){
			//normal 3 is nearest
			currentClustering[i] = 3;
		}

	}

}

//! Notes down to which mean a Normal is nearest in an euclidian sense. This means, normals are interpreted as points
void wEAssignNormalsToNearestMean(vector<int> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<Vertex*> &threeRandomlyChosenVertices){

	for(int i=0; i<verticesWithCurrentLabel.size(); i++){

		double distanceToNormal1 = wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(verticesWithCurrentLabel[i], threeRandomlyChosenVertices[0]);
		double distanceToNormal2 = wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(verticesWithCurrentLabel[i], threeRandomlyChosenVertices[1]);
		double distanceToNormal3 = wEComputeSquaredDistanceBetweenTwoNormalsTreatedAsVertices(verticesWithCurrentLabel[i], threeRandomlyChosenVertices[2]);

		if((distanceToNormal1 <= distanceToNormal2) && (distanceToNormal1 <= distanceToNormal3)) {
			//normal 1 is nearest
			currentClustering[i] = 1;
		} else if ((distanceToNormal2 <= distanceToNormal1) && (distanceToNormal2 <= distanceToNormal3)){
			//normal 2 is nearest
			currentClustering[i] = 2;
		} else if ((distanceToNormal3 <= distanceToNormal1) && (distanceToNormal3 <= distanceToNormal2)){
			//normal 3 is nearest
			currentClustering[i] = 3;
		}

	}

}


//! Computes three means of normals according to a given clustering
void wEComputeMeans(vector<int> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<vector<double>> &normalMeanComponents){

	double firstNormalMeanComponentX = 0.0;
	double firstNormalMeanComponentY = 0.0;
	double firstNormalMeanComponentZ = 0.0;

	int workedOnFirstCounter = 0;

	double secondNormalMeanComponentX = 0.0;
	double secondNormalMeanComponentY = 0.0;
	double secondNormalMeanComponentZ = 0.0;

	int workedOnSecondCounter = 0;

	double thirdNormalMeanComponentX = 0.0;
	double thirdNormalMeanComponentY = 0.0;
	double thirdNormalMeanComponentZ = 0.0;

	int workedOnThirdCounter = 0;

	for(int i=0; i<currentClustering.size(); i++){

		if(currentClustering[i] == 1){

			firstNormalMeanComponentX += verticesWithCurrentLabel[i]->getNormalX();
			firstNormalMeanComponentY += verticesWithCurrentLabel[i]->getNormalY();
			firstNormalMeanComponentZ += verticesWithCurrentLabel[i]->getNormalZ();

			workedOnFirstCounter++;

		} else if (currentClustering[i] == 2){

			secondNormalMeanComponentX += verticesWithCurrentLabel[i]->getNormalX();
			secondNormalMeanComponentY += verticesWithCurrentLabel[i]->getNormalY();
			secondNormalMeanComponentZ += verticesWithCurrentLabel[i]->getNormalZ();

			workedOnSecondCounter++;

		} else if (currentClustering[i] == 3) {

			thirdNormalMeanComponentX += verticesWithCurrentLabel[i]->getNormalX();
			thirdNormalMeanComponentY += verticesWithCurrentLabel[i]->getNormalY();
			thirdNormalMeanComponentZ += verticesWithCurrentLabel[i]->getNormalZ();

			workedOnThirdCounter++;

		}
	}

	//average

	firstNormalMeanComponentX = firstNormalMeanComponentX/workedOnFirstCounter;
	firstNormalMeanComponentY = firstNormalMeanComponentY/workedOnFirstCounter;
	firstNormalMeanComponentZ = firstNormalMeanComponentZ/workedOnFirstCounter;

	secondNormalMeanComponentX = secondNormalMeanComponentX/workedOnSecondCounter;
	secondNormalMeanComponentY = secondNormalMeanComponentY/workedOnSecondCounter;
	secondNormalMeanComponentZ = secondNormalMeanComponentZ/workedOnSecondCounter;

	thirdNormalMeanComponentX = thirdNormalMeanComponentX/workedOnThirdCounter;
	thirdNormalMeanComponentY = thirdNormalMeanComponentY/workedOnThirdCounter;
	thirdNormalMeanComponentZ = thirdNormalMeanComponentZ/workedOnThirdCounter;

	//write in vector

	normalMeanComponents[0][0] = firstNormalMeanComponentX;
	normalMeanComponents[0][1] = firstNormalMeanComponentY;
	normalMeanComponents[0][2] = firstNormalMeanComponentZ;

	normalMeanComponents[1][0] = secondNormalMeanComponentX;
	normalMeanComponents[1][1] = secondNormalMeanComponentY;
	normalMeanComponents[1][2] = secondNormalMeanComponentZ;

	normalMeanComponents[2][0] = thirdNormalMeanComponentX;
	normalMeanComponents[2][1] = thirdNormalMeanComponentY;
	normalMeanComponents[2][2] = thirdNormalMeanComponentZ;

}


//! computes the shortest distance between a point (arbPoint) and a line defined by two points lying on the line
//! TODO could be rewritten using a class or other helper methods
//! currently uses the square root and I don't like it
double wEComputeShortestDistanceBetweenPointAndRay(Vertex* &arbPoint, Vertex* &point1RayStart, Vertex* &point2OnRay){

	if((arbPoint!=point1RayStart)&&(arbPoint!=point2OnRay)){

		double rayVectorComponentX = point2OnRay->getX() - point1RayStart->getX();
		double rayVectorComponentY = point2OnRay->getY() - point1RayStart->getY();
		double rayVectorComponentZ = point2OnRay->getZ() - point1RayStart->getZ();

		double helperVectorComponentX = arbPoint->getX() - point1RayStart->getX();
		double helperVectorComponentY = arbPoint->getY() - point1RayStart->getY();
		double helperVectorComponentZ = arbPoint->getZ() - point1RayStart->getZ();

		double dotProduct1 =	rayVectorComponentX * helperVectorComponentX +
								rayVectorComponentY * helperVectorComponentY +
								rayVectorComponentZ * helperVectorComponentZ;

		double dotProduct2 =	rayVectorComponentX * rayVectorComponentX +
								rayVectorComponentY * rayVectorComponentY +
								rayVectorComponentZ * rayVectorComponentZ;

		double parameterOfProjection = dotProduct1 / dotProduct2;

		//the case, where the parameterOfProjection is < 0, we don't want.
		if(parameterOfProjection < 0){
			return sqrt(pow(helperVectorComponentX,2.0) + pow(helperVectorComponentY,2.0) + pow(helperVectorComponentZ,2.0));
		} else {

			double projectionComponentX = arbPoint->getX() - (point1RayStart->getX() + parameterOfProjection * rayVectorComponentX);
			double projectionComponentY = arbPoint->getY() - (point1RayStart->getY() + parameterOfProjection * rayVectorComponentY);
			double projectionComponentZ = arbPoint->getZ() - (point1RayStart->getZ() + parameterOfProjection * rayVectorComponentZ);

			return sqrt(pow(projectionComponentX,2.0) + pow(projectionComponentY,2.0) + pow(projectionComponentZ,2.0));
		}
	} else {
		//in case the arbitrary point is one of the vertices given to construct the line
		return 0.0;
	}
}

/*
//! computes the shortest distance between a point (arbPoint) and a line defined by two points lying on the line
//! TODO could be rewritten using a class or other helper methods
//! currently uses the square root and I don't like it
double wEComputeShortestDistanceBetweenPointAndLine(Vertex* &arbPoint, Vertex* &point1OnLine, Vertex* &point2OnLine){

	if((arbPoint!=point1OnLine)&&(arbPoint!=point2OnLine)){

		double lineVectorComponentX = point2OnLine->getX() - point1OnLine->getX();
		double lineVectorComponentY = point2OnLine->getY() - point1OnLine->getY();
		double lineVectorComponentZ = point2OnLine->getZ() - point1OnLine->getZ();

		double helperVectorComponentX = arbPoint->getX() - point1OnLine->getX();
		double helperVectorComponentY = arbPoint->getY() - point1OnLine->getY();
		double helperVectorComponentZ = arbPoint->getZ() - point1OnLine->getZ();

		//the magnitude of the cross product of the vectors gives the area of the created parallelogram
		//its height can be calculated by deviding the area by the distance of the points on the line

		//magnitude of the cross product of lineVector and helperVector
		double crossProductComponentX = (lineVectorComponentY * helperVectorComponentZ) - (lineVectorComponentZ * helperVectorComponentY);
		double crossProductComponentY = (lineVectorComponentZ * helperVectorComponentX) - (lineVectorComponentX * helperVectorComponentZ);
		double crossProductComponentZ = (lineVectorComponentX * helperVectorComponentY) - (lineVectorComponentY * helperVectorComponentX);

		//maybe sqrt can be omitted
		double parallelogramArea = sqrt(pow(crossProductComponentX, 2.0) + pow(crossProductComponentY, 2.0) + pow(crossProductComponentZ, 2.0));
		//maybe sqrt can be omitted
		double lineSegmentLength = sqrt(pow(lineVectorComponentX, 2.0) + pow(lineVectorComponentY, 2.0) + pow(lineVectorComponentZ, 2.0));

		return (parallelogramArea / lineSegmentLength);
	} else {
		//in case the arbitrary point is one of the vertices given to construct the line
		return 0.0;
	}

}
*/
/*
//! UNUSED Determines on which border of 2 clusters a border vertex lies
void wEGetBorderGroupFromVertexByFeatureVector(Vertex* vertexInQuestion,int &foundBordergroup){

	double borderTo1;
	vertexInQuestion->getFeatureElement(21, &borderTo1);
	double borderTo2;
	vertexInQuestion->getFeatureElement(22, &borderTo2);
	double borderTo3;
	vertexInQuestion->getFeatureElement(23, &borderTo3);

	if((borderTo1 > 0) && (borderTo2 > 0)){
		foundBordergroup = 12;
	} else if ((borderTo2 > 0) && (borderTo3 > 0)){
		foundBordergroup = 23;
	} else if ((borderTo3 > 0) && (borderTo1 > 0)){
		foundBordergroup = 31;
	}

}
*/

//! Determines on which border of 2 clusters a border vertex lies
void wEGetBorderGroupFromVertexByFeatureVector(Vertex* &finalLineVertex1, Vertex* &finalLineVertex2, Vertex* &finalLineVertex3,
				Vertex* &finalTetraederVertexGroup12, Vertex* &finalTetraederVertexGroup23, Vertex* &finalTetraederVertexGroup31){

	double borderGroup1;
	finalLineVertex1->getFeatureElement(21, &borderGroup1);

	double borderGroup2;
	finalLineVertex2->getFeatureElement(21, &borderGroup2);

	double borderGroup3;
	finalLineVertex3->getFeatureElement(21, &borderGroup3);

	if(round(borderGroup1) == 12){
		finalTetraederVertexGroup12 = finalLineVertex1;
	} else if (round(borderGroup1) == 23){
		finalTetraederVertexGroup23 = finalLineVertex1;
	} else if (round(borderGroup1) == 31){
		finalTetraederVertexGroup31 = finalLineVertex1;
	}

	if(round(borderGroup2) == 12){
		finalTetraederVertexGroup12 = finalLineVertex2;
	} else if (round(borderGroup2) == 23){
		finalTetraederVertexGroup23 = finalLineVertex2;
	} else if (round(borderGroup2) == 31){
		finalTetraederVertexGroup31 = finalLineVertex2;
	}

	if(round(borderGroup3) == 12){
		finalTetraederVertexGroup12 = finalLineVertex3;
	} else if (round(borderGroup3) == 23){
		finalTetraederVertexGroup23 = finalLineVertex3;
	} else if (round(borderGroup3) == 31){
		finalTetraederVertexGroup31 = finalLineVertex3;
	}

}


//! Calculates the squared distance between a projection point on a line (the result of projecting a point on a line) and a certain point on the line
//! To determine the point on the line this formula is used: https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line, Vector Formulation
void wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(Vertex* &arbPoint, Vertex* &point1OnLineTetraederTop, Vertex* &point2OnLine, double &computedSquaredDistance){

	if((arbPoint != point1OnLineTetraederTop) && (arbPoint != point2OnLine)){

		double vector1ComponentX = point1OnLineTetraederTop->getX() - arbPoint->getX();
		double vector1ComponentY = point1OnLineTetraederTop->getY() - arbPoint->getY();
		double vector1ComponentZ = point1OnLineTetraederTop->getZ() - arbPoint->getZ();

		double vector2ComponentX = point2OnLine->getX() - point1OnLineTetraederTop->getX();
		double vector2ComponentY = point2OnLine->getY() - point1OnLineTetraederTop->getY();
		double vector2ComponentZ = point2OnLine->getZ() - point1OnLineTetraederTop->getZ();

		//vector2 needs to be a normal
		double normalizationFactor = sqrt(pow(vector2ComponentX,2.0) + pow(vector2ComponentY,2.0) + pow(vector2ComponentZ,2.0));
		vector2ComponentX = vector2ComponentX/normalizationFactor;
		vector2ComponentY = vector2ComponentY/normalizationFactor;
		vector2ComponentZ = vector2ComponentZ/normalizationFactor;


		double dotProduct1 = vector1ComponentX * vector2ComponentX + vector1ComponentY * vector2ComponentY + vector1ComponentZ * vector2ComponentZ;

		double projectedPointOnLineComponentX = arbPoint->getX() + vector1ComponentX - (dotProduct1 * vector2ComponentX);
		double projectedPointOnLineComponentY = arbPoint->getY() + vector1ComponentY - (dotProduct1 * vector2ComponentY);
		double projectedPointOnLineComponentZ = arbPoint->getZ() + vector1ComponentZ - (dotProduct1 * vector2ComponentZ);


		double distanceComponentX = projectedPointOnLineComponentX - point1OnLineTetraederTop->getX();
		double distanceComponentY = projectedPointOnLineComponentY - point1OnLineTetraederTop->getY();
		double distanceComponentZ = projectedPointOnLineComponentZ - point1OnLineTetraederTop->getZ();
/*
		double distanceComponentX = dotProduct1/dotProduct2 * vector2ComponentX;
		double distanceComponentY = dotProduct1/dotProduct2 * vector2ComponentY;
		double distanceComponentZ = dotProduct1/dotProduct2 * vector2ComponentZ;
*/
		computedSquaredDistance = pow(distanceComponentX, 2.0) + pow(distanceComponentY, 2.0) + pow(distanceComponentZ, 2.0);

	} else if (arbPoint == point1OnLineTetraederTop){
		computedSquaredDistance = 0.0;
	} else {

		computedSquaredDistance = 	pow(point1OnLineTetraederTop->getX()-point2OnLine->getX(), 2.0) +
									pow(point1OnLineTetraederTop->getY()-point2OnLine->getY(), 2.0) +
									pow(point1OnLineTetraederTop->getZ()-point2OnLine->getZ(), 2.0);

	}

}


double wEComputeTetraederHeight(std::vector<Vertex*> &foundTetraeder){

	//every found tetraeder should consist of 4 vertices
	//this is checked for the sake of implementation
	if(foundTetraeder.size() == 4){

		double vector1ComponentX = foundTetraeder[2]->getX() - foundTetraeder[1]->getX();
		double vector1ComponentY = foundTetraeder[2]->getY() - foundTetraeder[1]->getY();
		double vector1ComponentZ = foundTetraeder[2]->getZ() - foundTetraeder[1]->getZ();

		double vector2ComponentX = foundTetraeder[3]->getX() - foundTetraeder[1]->getX();
		double vector2ComponentY = foundTetraeder[3]->getY() - foundTetraeder[1]->getY();
		double vector2ComponentZ = foundTetraeder[3]->getZ() - foundTetraeder[1]->getZ();

		double crossProductComponentX = (vector1ComponentY * vector2ComponentZ) - (vector1ComponentZ * vector2ComponentY);
		double crossProductComponentY = (vector1ComponentZ * vector2ComponentX) - (vector1ComponentX * vector2ComponentZ);
		double crossProductComponentZ = (vector1ComponentX * vector2ComponentY) - (vector1ComponentY * vector2ComponentX);

		double planeComponentD =	(crossProductComponentX * foundTetraeder[1]->getX() * -1.0) +
									(crossProductComponentY * foundTetraeder[1]->getY() * -1.0) +
									(crossProductComponentZ * foundTetraeder[1]->getZ() * -1.0);


		double distanceTopFromPlane =	( abs(	(crossProductComponentX * foundTetraeder[0]->getX()) +
												(crossProductComponentY * foundTetraeder[0]->getY()) +
												(crossProductComponentZ * foundTetraeder[0]->getZ()) +
												planeComponentD ) ) /
										( sqrt( pow(crossProductComponentX, 2.0) + pow(crossProductComponentY, 2.0) + pow(crossProductComponentZ, 2.0) ) );


		return distanceTopFromPlane;


	} else {
		return 0.0;
	}
}


//! Checks if a tetraeder is tall enough (considering the tetraeder top (the wedge vertex with highest MSII value) as top wedge from where to consider the height)
//! The found Tetraeder is made of 4 vertices: tetraeder top, vertex 1, vertex 2, vertex 3
bool wECheckTetraederHeight(vector<Vertex*> &foundTetraeder, double &minimumHeight){

	//every found tetraeder should consist of 4 vertices
	//this is checked for the sake of implementation
	if(foundTetraeder.size() == 4){

		double vector1ComponentX = foundTetraeder[2]->getX() - foundTetraeder[1]->getX();
		double vector1ComponentY = foundTetraeder[2]->getY() - foundTetraeder[1]->getY();
		double vector1ComponentZ = foundTetraeder[2]->getZ() - foundTetraeder[1]->getZ();

		double vector2ComponentX = foundTetraeder[3]->getX() - foundTetraeder[1]->getX();
		double vector2ComponentY = foundTetraeder[3]->getY() - foundTetraeder[1]->getY();
		double vector2ComponentZ = foundTetraeder[3]->getZ() - foundTetraeder[1]->getZ();

		double crossProductComponentX = (vector1ComponentY * vector2ComponentZ) - (vector1ComponentZ * vector2ComponentY);
		double crossProductComponentY = (vector1ComponentZ * vector2ComponentX) - (vector1ComponentX * vector2ComponentZ);
		double crossProductComponentZ = (vector1ComponentX * vector2ComponentY) - (vector1ComponentY * vector2ComponentX);

		double planeComponentD =	(crossProductComponentX * foundTetraeder[1]->getX() * -1.0) +
									(crossProductComponentY * foundTetraeder[1]->getY() * -1.0) +
									(crossProductComponentZ * foundTetraeder[1]->getZ() * -1.0);


		double distanceTopFromPlane =	( abs(	(crossProductComponentX * foundTetraeder[0]->getX()) +
												(crossProductComponentY * foundTetraeder[0]->getY()) +
												(crossProductComponentZ * foundTetraeder[0]->getZ()) +
												planeComponentD ) ) /
										( sqrt( pow(crossProductComponentX, 2.0) + pow(crossProductComponentY, 2.0) + pow(crossProductComponentZ, 2.0) ) );


		if(distanceTopFromPlane > minimumHeight){
			return true;
		} else {
			return false;
		}


	} else {
		return false;
	}
}

//! sqrt in calculation can probably be omitted? NO, I need vectors of Length 1
void checkTriangleFaceOrientation(vector<vector<Vertex*>> &extractedTetraeders, vector<bool> &faceVerdict){

	for(int i=0; i<extractedTetraeders.size(); i++){
		//calculate cross product
		double vector1ComponentX = extractedTetraeders[i][2]->getX() - extractedTetraeders[i][1]->getX();
		double vector1ComponentY = extractedTetraeders[i][2]->getY() - extractedTetraeders[i][1]->getY();
		double vector1ComponentZ = extractedTetraeders[i][2]->getZ() - extractedTetraeders[i][1]->getZ();

		double vector2ComponentX = extractedTetraeders[i][3]->getX() - extractedTetraeders[i][1]->getX();
		double vector2ComponentY = extractedTetraeders[i][3]->getY() - extractedTetraeders[i][1]->getY();
		double vector2ComponentZ = extractedTetraeders[i][3]->getZ() - extractedTetraeders[i][1]->getZ();

		double crossProductComponentX = (vector1ComponentY*vector2ComponentZ) - (vector1ComponentZ*vector2ComponentY);
		double crossProductComponentY = (vector1ComponentZ*vector2ComponentX) - (vector1ComponentX*vector2ComponentZ);
		double crossProductComponentZ = (vector1ComponentX*vector2ComponentY) - (vector1ComponentY*vector2ComponentX);


		//normalize cross product
		double crossProductVectorLength = sqrt(pow(crossProductComponentX,2.0) + pow(crossProductComponentY,2.0) + pow(crossProductComponentZ,2.0));
		crossProductComponentX = crossProductComponentX / crossProductVectorLength;
		crossProductComponentY = crossProductComponentY / crossProductVectorLength;
		crossProductComponentZ = crossProductComponentZ / crossProductVectorLength;

		//calculate normal means

		double normalSumComponentX =	extractedTetraeders[i][1]->getNormalX() +
						extractedTetraeders[i][2]->getNormalX() +
						extractedTetraeders[i][3]->getNormalX();
		double normalSumComponentY =	extractedTetraeders[i][1]->getNormalY() +
						extractedTetraeders[i][2]->getNormalY() +
						extractedTetraeders[i][3]->getNormalY();
		double normalSumComponentZ =	extractedTetraeders[i][1]->getNormalZ() +
						extractedTetraeders[i][2]->getNormalZ() +
						extractedTetraeders[i][3]->getNormalZ();

		//normalize mean

		double normalSumLength = sqrt(pow(normalSumComponentX,2.0) + pow(normalSumComponentY,2.0) + pow(normalSumComponentZ,2.0));
		normalSumComponentX = normalSumComponentX/normalSumLength;
		normalSumComponentY = normalSumComponentY/normalSumLength;
		normalSumComponentZ = normalSumComponentZ/normalSumLength;

		//note down true, if difference of cross product to mean is smaller than
		//from -1 * cross product to mean
		double invertedCrossProductComponentX = crossProductComponentX * (-1.0);
		double invertedCrossProductComponentY = crossProductComponentY * (-1.0);
		double invertedCrossProductComponentZ = crossProductComponentZ * (-1.0);

		double distanceNormalToCrossProduct =	pow(normalSumComponentX-crossProductComponentX,2.0) +
							pow(normalSumComponentY-crossProductComponentY,2.0) +
							pow(normalSumComponentZ-crossProductComponentZ,2.0);
		double distanceNormalToInvertedCrossProduct =	pow(normalSumComponentX-invertedCrossProductComponentX,2.0) +
								pow(normalSumComponentY-invertedCrossProductComponentY,2.0) +
								pow(normalSumComponentZ-invertedCrossProductComponentZ,2.0);
		if(distanceNormalToCrossProduct < distanceNormalToInvertedCrossProduct){
			faceVerdict.push_back(true);
		} else{
			faceVerdict.push_back(false);
		}

	}


}


void wENormalizeValues(vector<double> &vectorOfDoubles, vector<double> &vectorOfNormalizedDoubles){

	//find minimum of the input values
	double foundMinValue = *min_element(begin(vectorOfDoubles), end(vectorOfDoubles));

	//find maximum of the input values
	double foundMaxValue = *max_element(begin(vectorOfDoubles), end(vectorOfDoubles));

	for(auto doubleValue: vectorOfDoubles){

		double normalizedDoubleValue = (doubleValue - foundMinValue)/(foundMaxValue - foundMinValue);
		vectorOfNormalizedDoubles.push_back(normalizedDoubleValue);
	}

}


//! Writes extracted Tetraeders into a .obj-file
//! .obj-file only holds vertex positions and face definitions
//! .obj-file does not hold vertex color information
void wEWriteExtractedTetraedersIntoFile(vector<vector<Vertex*>> &extractedTetraeders, string outputFileName){

	string customPrefix = outputFileName;
	string customRemark = "_extractedWedges";
	string customSuffix = ".obj";
	string tetraederOutputFile = customPrefix + customRemark + customSuffix;
	ofstream OutFile(tetraederOutputFile);

	double vertexCoordinateX;
	double vertexCoordinateY;
	double vertexCoordinateZ;

	if(OutFile.fail()){

		cout << "Unable to open file to write tetraeder data!";

	}
	else{

		OutFile << "# This file holds the extracted tetraeders." << endl;

		//loop over tetraeder
		for(int i=0; i < extractedTetraeders.size(); i++){

			//loop over Vertices
			for(int j=0; j < 4; j++){

				vertexCoordinateX = extractedTetraeders[i][j]->getX();
				vertexCoordinateY = extractedTetraeders[i][j]->getY();
				vertexCoordinateZ = extractedTetraeders[i][j]->getZ();

				//possibly shorten the accuracy down

				OutFile << "v "  << vertexCoordinateX << " " << vertexCoordinateY << " " << vertexCoordinateZ << endl;
			}

		}
		//vertices have been written down.

		//now 4 faces per Tetraeder are written down
		int fComp1;
		int fComp2;
		int fComp3;
		int fComp4;
		for(int t=0; t < extractedTetraeders.size(); t++){

			fComp1 = 1+t*4;
			fComp2 = 2+t*4;
			fComp3 = 3+t*4;
			fComp4 = 4+t*4;

			OutFile << "f " << fComp1 << " " << fComp2 << " " << fComp3 << endl;
			OutFile << "f " << fComp1 << " " << fComp2 << " " << fComp4 << endl;
			//OutFile << "f " << fComp1 << " " << fComp3 << " " << fComp4 << endl; //may be not counterclockwise
			OutFile << "f " << fComp1 << " " << fComp4 << " " << fComp3 << endl; //counterclockwise equivalent to line before
			OutFile << "f " << fComp2 << " " << fComp3 << " " << fComp4 << endl;

		}
	}
}


//! Writes extracted Tetraeders into a .ply-file
//! .ply-file holds vertex positions, face definitions and vertex color information
void wEWriteExtractedTetraedersAndMeshIntoFile(	vector<vector<Vertex*>> &extractedTetraeders,
												string outputFileName,
												vector<double> &vectorOfNormalizedDoubles,
												int doubleInterpretation,
												vector<Vertex*> &mVertices,
												vector<Face*> &mFaces){

	//cout << "Ply Method starts" << endl;

	//prepare face rotation
	vector<bool> faceVerdict;
	checkTriangleFaceOrientation(extractedTetraeders, faceVerdict);

	string customPrefix = outputFileName;
	string customRemark = "_extractedWedges";

	if(doubleInterpretation == 0){
		customRemark = "_extractedWedges";
	} else if(doubleInterpretation == 1){
		customRemark = "_extractedWedges_RANSACQualityVisualized";
	} else if(doubleInterpretation == 2){
		customRemark = "_extractedWedges_TetraederHeightVisualized";
	}




	string customSuffix = ".ply";
	string tetraederOutputFile = customPrefix + customRemark + customSuffix;
	ofstream OutFile(tetraederOutputFile);

	double vertexCoordinateX;
	double vertexCoordinateY;
	double vertexCoordinateZ;

	unsigned char vertexColorR;
	unsigned char vertexColorG;
	unsigned char vertexColorB;

	int intVertexColorR;
	int intVertexColorG;
	int intVertexColorB;

	if(OutFile.fail()){

		cout << "Unable to open file to write tetraeder data!";

	}
	else{


		OutFile << "ply" << endl;
		OutFile << "format ascii 1.0" << endl;
		OutFile << "comment mesh and extracted wedges" << endl;

		//Total Number of Vertices needs to be known
		//It is the Mesh's number of vertices + (number of Tetraeders * 4)

		int numberOfMeshVertices = mVertices.size();
		int numberOfTetraederVertices = extractedTetraeders.size() * 4;
		int numberOfAllVertices = numberOfMeshVertices + numberOfTetraederVertices;

		OutFile << "element vertex " << numberOfAllVertices << endl;
		OutFile << "property float x" << endl;
		OutFile << "property float y" << endl;
		OutFile << "property float z" << endl;

		//colors

		OutFile << "property uchar red" << endl;
		OutFile << "property uchar green" << endl;
		OutFile << "property uchar blue" << endl;

		//faces
		//Total Number of Faces needs to be known

		int numberOfMeshFaces = mFaces.size();
		int numberOfTetraederFaces = extractedTetraeders.size() * 4; //same number as vertices

		//Experimental solution instead of face rotation.
		//numberOfTetraederFaces = 2*numberOfTetraederFaces;

		int numberOfAllFaces = numberOfMeshFaces + numberOfTetraederFaces;



		OutFile << "element face " << numberOfAllFaces << endl;

		OutFile << "property list uchar int vertex_index" << endl; //that means, every face (consisting of 3 vertices) needs to have 3 in the beginning)

		OutFile << "end_header" << endl;

		//list every vertex
		//example: 0 0 0 255 255 255

		//cout << "meshV" << endl;

		//cout << mVertices.size() << endl;
		//cout << mVertices.at(0) << endl; //exists, so indices start at 0
		//cout << mVertices.at(1) << endl;

		//all mesh vertices
		for(int i=0; i<mVertices.size(); i++){

			Vertex* currentVertexP = mVertices.at(i);
			vertexCoordinateX = currentVertexP->getX();
			vertexCoordinateY = currentVertexP->getY();
			vertexCoordinateZ = currentVertexP->getZ();

			//colors seem to be off with this solution

			vertexColorR = currentVertexP->getR();
			vertexColorG = currentVertexP->getG();
			vertexColorB = currentVertexP->getB();

			intVertexColorR = vertexColorR;
			intVertexColorG = vertexColorG;
			intVertexColorB = vertexColorB;

			//original mesh colors used
			OutFile << vertexCoordinateX << " " << vertexCoordinateY << " " << vertexCoordinateZ << " " << intVertexColorR << " " << intVertexColorG << " " << intVertexColorB << endl;

		}

		//cout << "TetV" << endl;

		//all additional vertices, added by algorithms

		//cout << extractedTetraeders.size() << endl;

		for(int i=0; i<extractedTetraeders.size(); i++){

			//cout << extractedTetraeders[i].size() << endl;

			for(int j=0; j<4; j++){

				//Vertex* currentVertexET = extractedTetraeders[i][j];

				vertexCoordinateX = extractedTetraeders[i][j]->getX();
				vertexCoordinateY = extractedTetraeders[i][j]->getY();
				vertexCoordinateZ = extractedTetraeders[i][j]->getZ();

				//deciding the vertex color

				//in case no particular attribute was chosen
				if(doubleInterpretation == 0){

					cout << "No particular attribute was chosen to be visualized." << endl;

					intVertexColorR = 0;
					intVertexColorG = 200;
					intVertexColorB = 0;

				} else if(doubleInterpretation == 1){ // RANSAC Quality is to be visualized

					if(vectorOfNormalizedDoubles[i] < 0.4){
						intVertexColorR = (int)round( (0.4-vectorOfNormalizedDoubles[i])*200 );
					} else {
						intVertexColorR = 0;
					}

					//remember small values of the RANSACQuality are good, they represent small errors
					intVertexColorG = 200 - (int)round(vectorOfNormalizedDoubles[i]*200);


					intVertexColorB = 0;
				} else if(doubleInterpretation == 2){ //Tetraeder Height is to be visualized
					if(vectorOfNormalizedDoubles[i] < 0.4){
						intVertexColorR = (int)round( (0.4-vectorOfNormalizedDoubles[i])*200 );
					} else {
						intVertexColorR = 0;
					}

					//remember small values of the RANSACQuality are good, they represent small errors
					intVertexColorG = (int)round(vectorOfNormalizedDoubles[i]*200);


					intVertexColorB = 0;
				}



				//green is used, what does this comment mean?
				OutFile << vertexCoordinateX << " " << vertexCoordinateY << " " << vertexCoordinateZ << " " << intVertexColorR << " " << intVertexColorG << " " << intVertexColorB << endl;

			}

		}

		//cout << "meshFac" << endl;


		//list every face
		//remember: indices in ply start at 0
		//example: 3 0 1 2

		//all mesh faces
		for(int i=0; i<mFaces.size(); i++){

			Face* currentFace = mFaces[i];

			Vertex* faceVertexA = currentFace->getVertA();
			Vertex* faceVertexB = currentFace->getVertB();
			Vertex* faceVertexC = currentFace->getVertC();

			int vertexAInOr = faceVertexA->getIndexOriginal();
			int vertexBInOr = faceVertexB->getIndexOriginal();
			int vertexCInOr = faceVertexC->getIndexOriginal();

			OutFile << "3" << " " << vertexAInOr << " " << vertexBInOr << " " << vertexCInOr << endl;
		}

		//cout << "additionalFac" << endl;



		//all tetraeder faces
		//remember, first all original mesh vertices are named, then start the additional vertices

		int offsetToOriginalMeshVertices = mVertices.size();

		int fComp1;
		int fComp2;
		int fComp3;
		int fComp4;

		for(int i=0; i<extractedTetraeders.size(); i++){

			int vertexT1Index = offsetToOriginalMeshVertices + i*4;
			int vertexT2Index = offsetToOriginalMeshVertices + 1 + i*4;
			int vertexT3Index = offsetToOriginalMeshVertices + 2 + i*4;
			int vertexT4Index = offsetToOriginalMeshVertices + 3 + i*4;


			/*
			//experimental: ignore winding order, create two faces glued to eachother
			//nicht erfolgreich
			OutFile << "3" << " " << vertexT2Index << " " << vertexT3Index << " " << vertexT4Index << endl;
			OutFile << "3" << " " << vertexT3Index << " " << vertexT2Index << " " << vertexT1Index << endl;
			OutFile << "3" << " " << vertexT4Index << " " << vertexT3Index << " " << vertexT1Index << endl;
			OutFile << "3" << " " << vertexT2Index << " " << vertexT4Index << " " << vertexT1Index << endl;

			OutFile << "3" << " " << vertexT3Index << " " << vertexT2Index << " " << vertexT4Index << endl;
			OutFile << "3" << " " << vertexT2Index << " " << vertexT3Index << " " << vertexT1Index << endl;
			OutFile << "3" << " " << vertexT3Index << " " << vertexT4Index << " " << vertexT1Index << endl;
			OutFile << "3" << " " << vertexT4Index << " " << vertexT2Index << " " << vertexT1Index << endl;
			*/


			//check if face needs to be rotated

			if(faceVerdict[i]){
				//first: face not containing the tetraeder top
				OutFile << "3" << " " << vertexT2Index << " " << vertexT3Index << " " << vertexT4Index << endl;
				OutFile << "3" << " " << vertexT3Index << " " << vertexT2Index << " " << vertexT1Index << endl;
				OutFile << "3" << " " << vertexT4Index << " " << vertexT3Index << " " << vertexT1Index << endl;
				OutFile << "3" << " " << vertexT2Index << " " << vertexT4Index << " " << vertexT1Index << endl;
			} else {
				//first: face not containing the tetraeder top
				OutFile << "3" << " " << vertexT3Index << " " << vertexT2Index << " " << vertexT4Index << endl;
				OutFile << "3" << " " << vertexT2Index << " " << vertexT3Index << " " << vertexT1Index << endl;
				OutFile << "3" << " " << vertexT3Index << " " << vertexT4Index << " " << vertexT1Index << endl;
				OutFile << "3" << " " << vertexT4Index << " " << vertexT2Index << " " << vertexT1Index << endl;
			}





			//this face creation does not guarantee correct winding order
			/*
			OutFile << "3" << " " << vertexT1Index << " " << vertexT2Index << " " << vertexT3Index << endl;
			OutFile << "3" << " " << vertexT1Index << " " << vertexT2Index << " " << vertexT4Index << endl;
			OutFile << "3" << " " << vertexT1Index << " " << vertexT4Index << " " << vertexT3Index << endl;
			OutFile << "3" << " " << vertexT2Index << " " << vertexT3Index << " " << vertexT4Index << endl;
			*/

			//Decision one:
			//Concerning the face, that does not contain the tetraeder top
			//It is made up from the Vertices corresponding to:
			//vertexT2Index, vertexT3Index, vertexT4Index

		}
	}
}


//! Method to write extracted tetraeders into mesh
//! Per Tetraeder 14 doubles are collected
//! Information: RANSACQuality, TetraederHeight, then for 4 Vertices: X, Y, Z coordinate
void wEWriteExtractedTetraedersIntoMeshForCollection(	vector<vector<double>> &meshIntrinsicExtractedTetraeders,
														vector<vector<Vertex*>> &extractedTetraeders,
														vector<double> &RANSACQualities,
														vector<double> &TetraederHeights){

	//clear the vector
	meshIntrinsicExtractedTetraeders.clear();

	if( (extractedTetraeders.size()==RANSACQualities.size()) && (extractedTetraeders.size()==TetraederHeights.size()) ){

		cout << "Extracted tetraeders will be collected in mesh." << endl;

		for(int i=0; i<extractedTetraeders.size(); i++){

			vector<double> collectionOfTetraederInformation;

			//First RANSAC Quality and then Tetraeder Height is added
			collectionOfTetraederInformation.push_back(RANSACQualities[i]);
			collectionOfTetraederInformation.push_back(TetraederHeights[i]);

			//Collect for the Tetraeder for all 4 vertices the X, Y and Z coordinate
			for(int j=0; j < 4; j++){

				collectionOfTetraederInformation.push_back(extractedTetraeders[i][j]->getX());
				collectionOfTetraederInformation.push_back(extractedTetraeders[i][j]->getY());
				collectionOfTetraederInformation.push_back(extractedTetraeders[i][j]->getZ());

			}

			meshIntrinsicExtractedTetraeders.push_back(collectionOfTetraederInformation);
		}

	} else {

		cout << "Extracted tetraeders were not collected in mesh. An implementation error is the cause." << endl;

	}

}

/*
//! a hacky helper method, that will not end up in the final version
void wETempNoteDownExtractedTetraeders(vector<vector<Vertex*>> &extractedTetraeders, int numberOfVertices, int numberOfFaces, vector<double> &RANSACQuality){

	int VertexExtensionCounter = numberOfVertices;
	int FaceExtensionCounter = numberOfFaces;

	//collect all tetraeder VerticesOfFace
	for(int i=0; i<extractedTetraeders.size(); i++){

		vector<VertexOfFace> currentTetraeder;

		for(int j=0; j<4; j++){

			VertexExtensionCounter++;
			double tempX = extractedTetraeders[i][j]->getX();
			double tempY = extractedTetraeders[i][j]->getY();
			double tempZ = extractedTetraeders[i][j]->getZ();
			VertexOfFace tempVOF(VertexExtensionCounter, tempX, tempY, tempZ);

			//a first test to make the vertices red
			tempVOF.setRGB(100,0,0);

			currentTetraeder.push_back(tempVOF);



		}

		tempVOFCollection.push_back(currentTetraeder);
	}


	//collect all pointers to VerticesOfFace
	for(int i=0; i<tempVOFCollection.size(); i++){

		vector<VertexOfFace*> currentTetraederPs;
		for(int j=0; j<4; j++){
			VertexOfFace * tempVertexOfFaceP = &tempVOFCollection[i][j];

			currentTetraederPs.push_back(tempVertexOfFaceP);
		}

		tempVOFPCollection.push_back(currentTetraederPs);
	}

	//create faces

	for(int i=0; i<tempVOFPCollection.size(); i++){

		VertexOfFace * tempVOFP1 = tempVOFPCollection[i][0];
		VertexOfFace * tempVOFP2 = tempVOFPCollection[i][1];
		VertexOfFace * tempVOFP3 = tempVOFPCollection[i][2];
		VertexOfFace * tempVOFP4 = tempVOFPCollection[i][3];


		FaceExtensionCounter++;
		Face customFace1(FaceExtensionCounter, tempVOFP1, tempVOFP2, tempVOFP3);
		FaceExtensionCounter++;
		Face customFace2(FaceExtensionCounter, tempVOFP1, tempVOFP2, tempVOFP4);
		FaceExtensionCounter++;
		Face customFace3(FaceExtensionCounter, tempVOFP1, tempVOFP4, tempVOFP3);
		FaceExtensionCounter++;
		Face customFace4(FaceExtensionCounter, tempVOFP2, tempVOFP3, tempVOFP4);

		tempFCollection.push_back(customFace1);
		tempFCollection.push_back(customFace2);
		tempFCollection.push_back(customFace3);
		tempFCollection.push_back(customFace4);
	}

	//collect pointers to faces

	for(int i=0; i<tempFCollection.size(); i++){
		Face* tempFacePointer = &tempFCollection[i];

		tempFPCollection.push_back(tempFacePointer);

	}

	// TESTING
	//how to add vertex or face

	//int countingFurther = mVertices.size();

	//VertexOfFace testVert1(countingFurther+1, 4.0, -1.0, 2.0);
	//VertexOfFace testVert2(countingFurther+2, 8.0, 5.0, -2.0);
	//VertexOfFace testVert3(countingFurther+3, -4.0, 1.0, 6.0);

	//int countingFF = mFaces.size();

	//Face customFace(countingFF+1, testVert1, testVert2, testVert3)
	//VertexOfFace* pointy1 = &testVert1;
	//VertexOfFace* pointy2 = &testVert2;
	//VertexOfFace* pointy3 = &testVert3;

}*/


//copied from mesh.cpp, will be legacy soon
//TODO: could be slimlined and moved to vertex class
bool getSurroundingVerticesInOrder (list<Vertex*> &adjacentVertsInOrder, Vertex* &pi, bool printDebug) {

	if(printDebug) {
		cout << endl << "[Mesh::getSurroundingVerticesInOrder] pi: " << pi->getIndex() << endl;
	}

	set<Face*> facesVisited;
	set<Face*> facesAdjacent;
	pi->getFaces(&facesAdjacent);
	Face* nextFace = nullptr;
	Face* currentFace = (*facesAdjacent.begin());
	facesVisited.insert( currentFace );
	nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
	adjacentVertsInOrder.clear();

	if(printDebug) {
		cout << "[Mesh::getSurroundingVerticesInOrder] Vertex A,B,C: " << currentFace->getVertA()->getIndex() << "\t" <<
		        currentFace->getVertB()->getIndex() << "\t" << currentFace->getVertC()->getIndex() << endl;
		cout << "[Mesh::getSurroundingVerticesInOrder] size of facesAdjacent: " << facesAdjacent.size() << endl;
		cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID1: " << currentFace->getIndex() << endl;
	}

	if(pi == currentFace->getVertA()) {
		adjacentVertsInOrder.push_back(currentFace->getVertA());
		adjacentVertsInOrder.push_back(currentFace->getVertC());
		adjacentVertsInOrder.push_back(currentFace->getVertB());
	}
	if(pi== currentFace->getVertB()) {
		adjacentVertsInOrder.push_back(currentFace->getVertB());
		adjacentVertsInOrder.push_back(currentFace->getVertC());
		adjacentVertsInOrder.push_back(currentFace->getVertA());
	}
	if(pi == currentFace->getVertC()) {
		adjacentVertsInOrder.push_back(currentFace->getVertC());
		adjacentVertsInOrder.push_back(currentFace->getVertA());
		adjacentVertsInOrder.push_back(currentFace->getVertB());
	}

	while( nextFace != nullptr ) {
		currentFace = nextFace;
		nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
		facesVisited.insert( currentFace );

		if(printDebug) {
			cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID2: " << currentFace->getIndex() << endl;
		}

		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertA()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_back(currentFace->getVertA());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertB()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_back(currentFace->getVertB());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertC()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_back(currentFace->getVertC());
		}
	}

	// in case of a border vertex we also have to dance around in the other direction:
	// in this case, the vertices are pushed to the beginning, to ensure the correct order in the list
	currentFace = (*facesAdjacent.begin());
	nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
	    if(printDebug) {
			cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID3: " << currentFace->getIndex() << endl;
		}

		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertA()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertA());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertB()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertB());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertC()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertC());
		}

		while( nextFace != nullptr ) {
		currentFace = nextFace;
		nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
		facesVisited.insert( currentFace );

		if(printDebug) {
			cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID4: " << currentFace->getIndex() << endl;
		}

		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertA()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertA());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertB()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertB());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertC()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertC());
		}
	}

	adjacentVertsInOrder.erase(std::remove(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), pi), adjacentVertsInOrder.end());

	if(printDebug) {
		cout << "[Mesh::getSurroundingVerticesInOrder] size of adjacentVertsInOrder: " << adjacentVertsInOrder.size() << endl;
		list<Vertex*>::iterator iter;
		for(iter=adjacentVertsInOrder.begin(); iter!=adjacentVertsInOrder.end();++iter) {
			cout << (*iter)->getIndex() << endl;
		}
	}

	return true;
}


//! builds up a structure to look up vertex connectivity
void wEBuildVertexNeighbourLookUpStructure(vector<Vertex*> &mVertices, map<Vertex*,set<Vertex*>> &vertexNeighbourLookUp){

	for(auto pVertexPreC : mVertices){

		list<Vertex*> adjacentVerticesPreC;
		getSurroundingVerticesInOrder(adjacentVerticesPreC, pVertexPreC, false);

		set<Vertex*> retrievedNeighbours;
		for(auto& adjacentVertexPreC : adjacentVerticesPreC){
			retrievedNeighbours.insert(adjacentVertexPreC);
		}
		vertexNeighbourLookUp.insert(pair<Vertex*,set<Vertex*>>(pVertexPreC, retrievedNeighbours) ) ;
	}
	/*
	vector<vector<Vertex*>> vertexNeighbourLookUp(mVertices.size());

	for(auto pVertexPreC : mVertices){

		list<Vertex*> adjacentVerticesPreC;
		getSurroundingVerticesInOrder(adjacentVerticesPreC, pVertexPreC, false);

		vector<Vertex*> retrievedNeighbours;
		for(auto& adjacentVertexPreC : adjacentVerticesPreC){
			retrievedNeighbours.push_back(adjacentVertexPreC);
		}

		vertexNeighbourLookUp[pVertexPreC->getIndexOriginal()] = retrievedNeighbours;
	}*/

}
/*
bool temporaryTxTCreator(vector<Vertex*> &mVertices, map<Vertex*,set<Vertex*>> &vertexNeighbourLookUp){

	string customPrefix = "experimental";
	string customRemark = "_similarToPly";
	string customSuffix = ".txt";
	string tetraederOutputFile = customPrefix + customRemark + customSuffix;
	ofstream OutFile(tetraederOutputFile);

	int  vertexIdentifier;
	double vertexCoordinateX;
	double vertexCoordinateY;
	double vertexCoordinateZ;

	double MSIIvalue;

	vector<Vertex*> VectorElements;


	if(OutFile.fail()){

		cout << "Unable to open file to write tetraeder data!";

	}
	else{

		//OutFile << "# This file holds the extracted tetraeders." << endl;

		//loop over tetraeder
		for(auto currentVertex : mVertices){

			vertexIdentifier = currentVertex->getIndexOriginal();

			vertexCoordinateX = currentVertex->getX();
			vertexCoordinateY = currentVertex->getY();
			vertexCoordinateZ = currentVertex->getZ();

			currentVertex->getFeatureElement(15, &MSIIvalue);

			//possibly shorten the accuracy down

			OutFile << "Vertex" << " " << vertexIdentifier << " "  << MSIIvalue << " " << vertexCoordinateX << " " << vertexCoordinateY << " " << vertexCoordinateZ << endl;

		}

		for(auto const& lookUpElement : vertexNeighbourLookUp){

			OutFile << "Face" << " ";

			vertexIdentifier = lookUpElement.first->getIndexOriginal();

			OutFile << vertexIdentifier << " ";

			//VectorElements = lookUpElement.second;

			for(auto pVertex : lookUpElement.second){

				OutFile << pVertex->getIndexOriginal() << " ";
			}

			OutFile << endl;
		}
	}
}
*/

//methods called in mesh.cpp


//! an experimental non maximum suppression method.
//! Takes as input the distance, how far NMS should look (the frontier will progress until vertices are too far away)
//! will enlarge the feature vectors of all vertices to size 18
//! Data will be written at Feature Vector Position 17 (boolean if maximum) and 18 (if maximum, then averaged MSII)
bool experimentalSuppressNonMaxima(double &NMSDistance, vector<Vertex*> &mVertices){

	//these counters are used for feedback in the terminal
	int processedVertexCounter = 0;
	int maximaCounter = 0;

	//look up vector, will be cleared for every vertex.
	//vector<bool> checkVisited (mVertices.size());
	//look up set, will be cleared for every vertex
	set<Vertex*> checkVisited;

	//avoid using square root to speed things up
	double squaredDistance = NMSDistance * NMSDistance;

	//Create a faster look up structure to get neighbouring vertices.
	//The non maximum suppression relies on often accessing neighbours of vertices

	map<Vertex*,set<Vertex*>> vertexNeighbourLookUp;
	wEBuildVertexNeighbourLookUpStructure(mVertices, vertexNeighbourLookUp);

	//vertices that had a smaller MSII value than another vertex are collected here. They are no longer eligible to become local maxima themselves.
	set<Vertex*> succumbed;


	cout << "Lookup structure was established." << endl;


	//temporaryTxTCreator(mVertices, vertexNeighbourLookUp);

	/*
	vector<vector<Vertex*>> vertexNeighbourLookUp(mVertices.size());

	for(auto pVertexPreC : mVertices){

		list<Vertex*> adjacentVerticesPreC;
		getSurroundingVerticesInOrder(adjacentVerticesPreC, pVertexPreC, false);

		vector<Vertex*> retrievedNeighbours;
		for(auto& adjacentVertexPreC : adjacentVerticesPreC){
			retrievedNeighbours.push_back(adjacentVertexPreC);
		}

		vertexNeighbourLookUp[pVertexPreC->getIndexOriginal()] = retrievedNeighbours;
	}*/

	//loop over all vertices
	for(auto pVertex : mVertices){

		//initialize the checks for visited vertices with false
		//maybe use methods that already exist in vertex.cpp
		/*
		for (int i = 0; i < checkVisited.size() ; i++){
			checkVisited[i] = false;
			//maybe this works too: checkVisited.at(i) = false
		}*/

		//if the feature vector has less than 18 elements, resize it to length 18
		//the current concept is: use the feature vector and write computated values at position 17 and 18
		//resizing pads with zeros

		if(18 > pVertex->getFeatureVectorLen()){
			pVertex->resizeFeatureVector(18);
		}

		//temp Idea
		/*
		if(24 > pVertex->getFeatureVectorLen()){
			pVertex->resizeFeatureVector(24);
			pVertex->setFeatureElement(22, 0.0);
			pVertex->setFeatureElement(23, 0.0);
		}
		*/

		//used for relative feature vector element access
		//unsigned int currentFeatureVectorLength = pVertex->getFeatureVectorLen();

		//clear data that might sit at these positions, written their by previous calculations with different distances
		//Position 17 (index 16) shows if a vertex is a local maximum
		//Position 18 (index 17) includes the vertex's MSII-value if it is a local maximum
		//pVertex->setFeatureElement( (currentFeatureVectorLength-2) , 0.0);
		//pVertex->setFeatureElement( (currentFeatureVectorLength-1) , 0.0);

		//legacy, initialize with zero, position access deemed too absolute
		//however I like it better at the moment
		pVertex->setFeatureElement(16, 0.0);
		pVertex->setFeatureElement(17, 0.0);


		//per default, it is guessed not to be the (local) maximum
		bool isMax = false;
		//this boolean is used to abort loops quickly if neighbours with larger MSII values are found
		bool biggerValueFound = false;

		//acquire the MSII-value for this vertex
		double MSIIcomponent16;
		//pVertex->getFeatureElement( (currentFeatureVectorLength-3) , &MSIIcomponent16);

		//legacy, position access deemed too absolute
		//however I like it more
		pVertex->getFeatureElement(15, &MSIIcomponent16);
		//double distanceModifier = 1.0 - MSIIcomponent16; //find out, if 1.0 or maybe 1.1 is better

		/*
		double neverWillBeMax;
		pVertex->getFeatureElement(22, &neverWillBeMax);
		*/

		if((MSIIcomponent16 > temporaryNMSParameter) && !(succumbed.find(pVertex) != succumbed.end())/*&& (neverWillBeMax < 0.5)*/){
		//if(MSIIcomponent16 > temporaryNMSParameter){

			//Vertices, which will be the input for every search
			queue<Vertex*> marchingFrontier;

			marchingFrontier.push(pVertex);

			//if the list marchingFrontier is empty, there are no vertices to be checked.

			//as long as there are eligible candidates, perform the algorithm
			//only perform the algorithm when no bigger MSII value than the one from the current vertex (pVertex) has been found
			while(!marchingFrontier.empty() && !biggerValueFound){

				set<Vertex*> adjacentVertices = vertexNeighbourLookUp[marchingFrontier.front()];
				//vector<Vertex*> adjacentVertices = vertexNeighbourLookUp[marchingFrontier.front()->getIndexOriginal()];
				//getSurroundingVerticesInOrder(adjacentVertsInOrder, marchingFrontier.front(), false);

				//vertices are marked as visited, when they they are asked for their neighbours
				checkVisited.insert(marchingFrontier.front());

				//after the front vertex of the queue was marked as visitied, it is popped
				marchingFrontier.pop();

				//loop over the new found neighbours
				//for(auto adjacentVertex : adjacentVertsInOrder){
				//set<Vertex*>::iterator it;
				for(auto adjacentVertex : adjacentVertices){

					//only perform the algorithm, if no bigger value was found AND the candidate has not been visited yet
					if(!biggerValueFound && !(checkVisited.find(adjacentVertex) != checkVisited.end())  ){

						bool stillInReach = false;


						//Highly experimental to quicken searches is used in the next expression

						//helper method for distance calculation is used
						if( wEComputeSquaredDistanceBetweenTwoVertices(adjacentVertex, pVertex) <= (squaredDistance)){

							stillInReach = true;

						}

						//consider only vertices that are within the reach specified by the user
						if(stillInReach){

							double adjacentVertexsMSIIcomponent16;
							//adjacentVertex->getFeatureElement( (currentFeatureVectorLength-3) , &adjacentVertexsMSIIcomponent16);

							//legacy, position access deemed too absolute
							//however I like it more
							adjacentVertex->getFeatureElement(15, &adjacentVertexsMSIIcomponent16);

							// if a bigger MSII value than the one of the current vertex (pVertex) was found, pVertex is not the (lokal) maximum
							if(MSIIcomponent16 <= adjacentVertexsMSIIcomponent16){

								biggerValueFound = true;
								isMax = false; //not necessary

								//pVertex->setFeatureElement(22, 1.0); //temporary
								//pVertex->setFeatureElement(23, NMSDistance); //temporary

							}else{ //so it still seems to be the (local) maximum

								isMax = true; //the current Vertex temporarily seems to be a maximum

								//the vertex with the smaller MSII value is no longer eligible to become a local maximum
								succumbed.insert(adjacentVertex);

								//the new found neighbour doesn't have a bigger MSII value, so it is added to the front (put at the end of the queue
								//maybe his neighbours have a bigger MSII value
								marchingFrontier.push(adjacentVertex);

							}
						}
					}
				}
			}
			//if after the while loop the boolean isMax was set to true and remained true, the current vertex pVertex is a maximum
		}

		//if the current vertex is the maximum
		//write at Feature Vector Position 17 a pseudo boolean state.
		//write at Feature Vector Position 18 its MSII value.
		if(isMax){

			//pVertex->setFeatureElement( (currentFeatureVectorLength-2) , 1.0);
			//pVertex->setFeatureElement( (currentFeatureVectorLength-1) , MSIIcomponent16);

			//legacy, position access deemed to absolute
			//however I like it better
			pVertex->setFeatureElement(16, 1.0);
			pVertex->setFeatureElement(17, MSIIcomponent16);


			maximaCounter++;

		}
		//this else-block is important to update the values calculated by this method for example in case another distance is chosen.
		//without this else-block outdated values may remain in the feature vector.
		//alternatively the reset to zero could be implemented at the beginning of the method.
		else {

			//pVertex->setFeatureElement( (currentFeatureVectorLength-2) , 0.0);
			//pVertex->setFeatureElement( (currentFeatureVectorLength-1) , 0.0);

			//legacy, position access deemed to absolute
			//however I like it more
			pVertex->setFeatureElement(16, 0.0);
			pVertex->setFeatureElement(17, 0.0);


		}

		processedVertexCounter++;
		int totalNumberOfVertices = mVertices.size();

		if((processedVertexCounter % 10000) == 0){

			cout << processedVertexCounter << " of " << totalNumberOfVertices << " vertices have been processed until now." << endl;

		}

		checkVisited.clear();

	}// all vertices were looped over

	cout << "Given the radius of: " << NMSDistance << " these many MSII values (local maxima) survived non maximum suppression: " << maximaCounter << endl;

	return true;

}


//! An experimental Watershed method using values computed in the Non Maximum Suppression Method
//! will enlarge the feature vectors of all vertices to size 20
//! Data will be written at Feature Vector Position 19 (order of processing) and 20 (label)
bool experimentalComputeWatershed(double &watershedLimit, bool useBasinMerging, vector<Vertex*> &mVertices){

	double counterForFutureWork = 1.0;

	//prepare a look up vector to mark visited vertices
	set<Vertex*> checkVisited;

	/*
	//initialize the checks for visited vertices with false
	for (int i = 0; i < checkVisited.size() ; i++){
		checkVisited[i] = false;
		//maybe this works too: checkVisited.at(i) = false
	}
	*/

	//list to retrieve all vertices which are maxima
	//they have 1.0 at feature vector position 17 (index 16)
	list<Vertex*> foundMaxima;

	//labels start with 1 and go up to (the number of maxima)
	//double experimentalLabel = 1.0;

	int totalMaxima = 0;
	vector<double> labelsToBeGiven;

	//Structure to look up the number of vertices per label
	map<int, int> numberOfVerticesPerLabel;
	//Structure to look up neighbouring labels per label
	map<int, set<int>> neighbouringLabels;

	//find the number of labels to later give them in a random order
	for(auto pVertex: mVertices){
		double isLocalMaximum;
		pVertex->getFeatureElement(16, &isLocalMaximum);

		if(isLocalMaximum > 0.0){

			totalMaxima = totalMaxima + 1;
			labelsToBeGiven.push_back( (double)totalMaxima );

		}
	}

	// reorder the labels randomly
	unsigned timeBasedSeed = chrono::system_clock::now().time_since_epoch().count();
	shuffle(labelsToBeGiven.begin(), labelsToBeGiven.end(), default_random_engine(timeBasedSeed));


	//prepare data
	for(auto pVertex : mVertices){

		//if the feature vector has less than 20 elements, resize it to length 20
		if(20 > pVertex->getFeatureVectorLen())
		{
			pVertex->resizeFeatureVector(20);
		}

		//clear data, that might sit in the feature vectors, remaining from previous runs of the watershed algorithm

		pVertex->setFeatureElement(18, 0.0);
		pVertex->setFeatureElement(19, 0.0);

		double isLocalMaximum;
		pVertex->getFeatureElement(16, &isLocalMaximum);
		//more relative, less absolute
		//pVertex->getFeatureElement( (getVertexNr() - 4) , &isLocalMaximum);

		//add to list of maxima and mark as visited
		//in addition label the vertex
		//a grand counter increments for future work
		if(isLocalMaximum > 0.0){

			foundMaxima.push_back(pVertex);

			double experimentalLabel = labelsToBeGiven[0];
			labelsToBeGiven.erase(labelsToBeGiven.begin());

			pVertex->setFeatureElement(18, counterForFutureWork);
			pVertex->setFeatureElement(19, experimentalLabel);

			//Update the number of vertices per label
			numberOfVerticesPerLabel.insert(pair<int, int>((int)round(experimentalLabel), 1));

			/*
			int vertexIndex = pVertex->getIndexOriginal();
			checkVisited[vertexIndex] = true;
			*/
			checkVisited.insert(pVertex);

			counterForFutureWork = counterForFutureWork + 1.0;
			//experimentalLabel = experimentalLabel + 1.0;

			//cout << counterForFutureWork << " vertices have been processed." << endl;

		}
	}



	//after this step a list of maxima exists, they are labeled and marked as visited

	//these many clusters will exist
	//int numberOfClusters = foundMaxima.size();

	//the coast, as a map structure
	//this structure was chosen, because it provides ordering
	//the vertices will be ordered by their MSII value
	map<double, Vertex*> watershedCoast;
	//using a multimap may be safer so multiple vertices with the same MSII-value can be included

	//initialize the coast
	//this step can be integrated in the data preparation step, however maybe the separation is useful later
	for(auto& foundMaximum : foundMaxima){

		list<Vertex*> adjacentVertsInOrder;
		getSurroundingVerticesInOrder(adjacentVertsInOrder, foundMaximum, false);

		//retrieve the label, to label coast candidates
		double maximumLabel;
		foundMaximum->getFeatureElement(19, &maximumLabel);

		for(auto& coastCandidate : adjacentVertsInOrder){

			//int vertexIndex = coastCandidate->getIndexOriginal();
			double coastCandidateMSIIValue;
			coastCandidate->getFeatureElement(15, &coastCandidateMSIIValue);

			//only continue, if the vertex was not visited yet AND the vertex's MSII value is larger than the watershedLimit
			if( ( !(checkVisited.find(coastCandidate) != checkVisited.end()) ) && (coastCandidateMSIIValue > watershedLimit) ){

				double currentCandidateLabel;
				coastCandidate->getFeatureElement(19, &currentCandidateLabel);

				//label the vertex, if it has no label yet
				//labels start at 1.0, a value below means no label was given yet
				//should not occur here as local maxima would not initially not allow other maxima as neighbours. (Those maxima would have a label)
				if(currentCandidateLabel < 1.0){

					coastCandidate->setFeatureElement(19, maximumLabel);
					//Update the number of vertices per label
					numberOfVerticesPerLabel[(int)round(maximumLabel)] = numberOfVerticesPerLabel[(int)round(maximumLabel)] + 1;

					counterForFutureWork = counterForFutureWork + 1.0;

					//add the vertex to the coast
					//the coast is sorted by the MSII-Value



					//there might be a loss of vertices, if vertices have the same MSII value (at position 16 (index 15)) because the map does not allow multiple equal keys
					//the loss of vertices seems to not happen in training data

					watershedCoast.insert( pair<double, Vertex*>(coastCandidateMSIIValue, coastCandidate) );

					//DO NOT mark the vertex as visited
					//A Vertex gets marked as visited ONLY, when its neighbours have been retrieved

				}
				//no else clause here to find the label of already labeled vertices.

				/*
				//add the vertex to the coast
				//the coast is sorted by the MSII-Value

				double coastCandidateMSIIValue;
				coastCandidate->getFeatureElement(15, &coastCandidateMSIIValue);

				//there might be a loss of vertices, if vertices have the same MSII value (at position 16 (index 15)) because the map does not allow multiple equal keys
				watershedCoast.insert( pair<double, Vertex*>(coastCandidateMSIIValue, coastCandidate) );

				//DO NOT mark the vertex as visited
				//A Vertex is visited ONLY, when its neighbours have been retrieved
				*/
				//der Block wurde nach oben gezogen, sobald Nachbarn noch labelbar sind und gerade gelabeled werden, werden sie zur coast hinzugefuegt
			}
		}
	}

	//the following code is very similar to the code above, I consider rewriting this method

	//as long as the coast is not empty
	while(!watershedCoast.empty()){

		//take the Vertex with the largest MSII-value
		Vertex* VertexPWithLargestMSIIValue = watershedCoast.rbegin()->second;

		//store the key of that vertex for later use
		double VPWMSIIValue = watershedCoast.rbegin()->first;

		//that vertex already has a label
		//label its neighbours with that label, add them to the coast

		list<Vertex*> adjacentVertsInOrder;
		getSurroundingVerticesInOrder(adjacentVertsInOrder, VertexPWithLargestMSIIValue, false);

		//retrieve the label to label coast candidates
		double maximumLabel;
		VertexPWithLargestMSIIValue->getFeatureElement(19, &maximumLabel);

		for(auto& newCoastCandidate : adjacentVertsInOrder){

			//int candidateIndex = newCoastCandidate->getIndexOriginal();
			double coastVertexMSIIValue;
			newCoastCandidate->getFeatureElement(15, &coastVertexMSIIValue);

			//only continue, if the vertex was not visited yet AND the vertex's MSII value is larger than the watershedLimit
			if( ( !(checkVisited.find(newCoastCandidate) != checkVisited.end()) ) && (coastVertexMSIIValue > watershedLimit) ) {

				double currentCandidateLabel;
				newCoastCandidate->getFeatureElement(19, &currentCandidateLabel);

				//label the vertex, if it has no label yet
				//labels start at 1.0, a value below means no label was given yet
				if(currentCandidateLabel < 1.0){

					newCoastCandidate->setFeatureElement(19, maximumLabel);
					//Update the number of vertices per label
					numberOfVerticesPerLabel[(int)round(maximumLabel)] = numberOfVerticesPerLabel[(int)round(maximumLabel)] + 1;

				} else {
					//in case the vertex already has a label, note it down in the neighbouring labels structure
					neighbouringLabels[(int)round(maximumLabel)].insert((int)round(currentCandidateLabel));

				}

				//add the vertex to the coast
				//the coast is sorted by the MSII-Value



				//there might be a loss of vertices, if vertices have the same MSII value (at position 16 (index 15)) because the map does not allow multiple equal keys
				watershedCoast.insert( pair<double, Vertex*>(coastVertexMSIIValue, newCoastCandidate) );

			}
		}

		//the current vertex with the largest MSII value (maybe now not anymore) has been searched for neighbours
		//so mark it as visited and note down when it was treated

		VertexPWithLargestMSIIValue->setFeatureElement(18, counterForFutureWork);
		counterForFutureWork = counterForFutureWork + 1.0;

		//mark the Vertex as visited (after its neighbours were added to the coast)
		//int vertexIndex = VertexPWithLargestMSIIValue->getIndexOriginal();
		checkVisited.insert(VertexPWithLargestMSIIValue);

		//remove the vertex from the coast
		//DO NOT // watershedCoast.erase(prev(watershedCoast.end())); //THIS IS WRONG, meanwhile the highest element no longer has to be the vertex we looked at, because we inserted other vertices
		watershedCoast.erase(VPWMSIIValue);

		int totalNumberOfVertices = mVertices.size();

		if(((int)counterForFutureWork % 10000) == 0){

			cout << counterForFutureWork << " of " << totalNumberOfVertices << " vertices have been processed." << endl;

		}
	}

	cout << "The rest of the vertices were not eligible to be labeled, their MSII value was too low." << endl;
	//the limit results in vertices being not labeled because their MSIIvalue was too low

	cout << foundMaxima.size() << " maxima formed the base for the watershed algorithm." << endl;

	if(useBasinMerging){
		cout << "Basin merging was chosen to be applied" << endl;
		//postprocess in basin merging process
		//int postprocessingCounter = 0;

		//note that the neighbouring labels structure also holds the label x as neighbour of label x
		map<int, set<int>>::iterator ppit;

		//a structure is built to note down which basins have to merged
		map<int, int> mergeKeyIntoVal;
		//for(ppit = neighbouringLabels.begin(); ppit != neighbouringLabels.end(); ppit++){
		for(auto const& mapelement : neighbouringLabels){
			if(!mapelement.second.empty()){
				for(auto setelement : mapelement.second){
					if(numberOfVerticesPerLabel[setelement] != 0){
						double labelQuotient = 1.0 * numberOfVerticesPerLabel[mapelement.first] / numberOfVerticesPerLabel[setelement];
						if(labelQuotient < temporaryWaterParameter){
							//cout << numberOfVerticesPerLabel[mapelement.first] << " were deemed as too few." << endl;
							//in case label mapelement first has too few elements, it is noted down to be merged with label setelement
							mergeKeyIntoVal.insert(pair<int, int>(mapelement.first, setelement));
							//postprocessingCounter += 1;
						}
					}
				}
			}
		}



		//perform the post processing step of changing labels
		if(!mergeKeyIntoVal.empty()){
			cout << mergeKeyIntoVal.size() << " Labels will be rewritten." << endl;
			for(auto pVertex : mVertices){
				double vertexLabel;
				pVertex->getFeatureElement(19, &vertexLabel);
				//if a vertex has a label that should be exchanged by another, do it
				for(auto& mapelement : mergeKeyIntoVal){

					if((int)round(vertexLabel) == mapelement.first){
						double rewrittenLabel = 1.0 * mapelement.second;
						pVertex->setFeatureElement(19, rewrittenLabel);
					}
				}

			}
		} else{
			cout << "No labeled group was too small to be merged." << endl;
		}
	}else{
		cout << "Basin merging was chosen not to be applied." << endl;
	}



	//cout << "The watershed algorithm did not reach " << notLabeledYet << " vertices." << endl;
	//cout << "Postprocessing gave " << postProcessingCouldLabel << " of these vertices a label." << endl;

	return true;

}


//! An experimental clustering method
//! It does not use in-cluster variance to judge how good the clustering is
//! Instead it relies on multiple iterations and mean adjusting
//! Even if the user specifies 0 iteration, one iteration is done
//! Will enlarge the feature vectors of all vertices to size 21
//! Data will be written at Feature Vector Position 21 (sub clustering)
bool experimentalComputeClustering(int numberOfIterations, vector<Vertex*> &mVertices){

	//will note down the number of different labels used
	//is likely the same a the number of maxima that non maximum suppression produced
	int numberOfLabels = 0.0;

	int verticesWithoutLabel = 0;

	//prepare data
	//find the number of labels and the number of vertices, which were not labeled
	for(auto pVertex : mVertices){

		//if the feature vector has less than 21 elements, resize it to length 21
		if(21 > pVertex->getFeatureVectorLen()){

			pVertex->resizeFeatureVector(21);

		}

		//clear data, that might sit in the feature vectors, remaining from previous runs of the clustering algorithm
		pVertex->setFeatureElement(20, 0.0);

		double givenLabel;
		pVertex->getFeatureElement(19, &givenLabel);

		if(givenLabel < 1.0){

			//a vertex was not labeled in the watershed step
			verticesWithoutLabel++;

		}else{ //find out how many labels are used

			if(givenLabel > numberOfLabels){

				numberOfLabels = givenLabel;

			}
		}
	}

	//number of labeled groups is now known
	//it is noted down as int numberOfLabels

	//loop over every cluster
	for(int labelIterator=1;labelIterator<=numberOfLabels;labelIterator++){

		vector<Vertex*> verticesWithCurrentLabel;

		//add vertices of the currently investigated label to a big vector
		for(auto investigatedVertex : mVertices){

			double foundLabel;
			investigatedVertex->getFeatureElement(19, &foundLabel);

			if(labelIterator == (int)round(foundLabel)){

				verticesWithCurrentLabel.push_back(investigatedVertex);

			}
		}
		//The vector verticesWithCurrentLabel now holds all the vertices which will be used for clustering
		//there already is some randomness involved in creating this vector
		//however another shuffling (resulting in a random selection of 3 future cluster means) is done

		//clustering is only performed, if the verticesWithCurrentLabel vector holds enough vertices to begin with, which is 3 or more
		if(verticesWithCurrentLabel.size() >= 3){

			vector<Vertex*> threeRandomlyChosenVertices;
			//the random selection is done by a helper method
			wERandomlyChooseVerticesFromVector(verticesWithCurrentLabel, threeRandomlyChosenVertices, 3);

			//the currentClustering will represent the solution after the given number of iterations.
			vector<int> currentClustering (verticesWithCurrentLabel.size());

			//the assignment to the nearest mean is done by a helper method
			wEAssignNormalsToNearestMean(currentClustering, verticesWithCurrentLabel, threeRandomlyChosenVertices);

			//the initial cluster centers
			//Vertex* clusterCNr1 = threeRandomlyChosenVertices[0];
			//Vertex* clusterCNr2 = threeRandomlyChosenVertices[1];
			//Vertex* clusterCNr3 = threeRandomlyChosenVertices[2];

			vector<vector<double>> normalMeans = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};

			for(int iteration=0;iteration<numberOfIterations;iteration++){

				//The mean computation is done by a helper method
				wEComputeMeans(currentClustering, verticesWithCurrentLabel, normalMeans);

				//Determining to which mean a normal is closest is done by a helper method
				wEAssignNormalsToNearestMean(currentClustering, verticesWithCurrentLabel, normalMeans);

			}
			//Clustering did as many iterations as the user wanted.

			//The final clustering is noted down per vertex

			//write the clustering into the feature vector
			for(int i = 0;i<verticesWithCurrentLabel.size();i++){

				Vertex* currentVertex = verticesWithCurrentLabel[i];
				currentVertex->setFeatureElement(20, (double)currentClustering[i]);

			}
		}
	}

	cout << "The clustering algorithm found " << verticesWithoutLabel << " vertices without label." << endl;
	cout << "The clustering algorithm worked on " << numberOfLabels  << " labels, k-means found 3 clusters in each labeled group." << endl;

	return true;

}

//! RANSAC algorithm that fits a tetraeder (into a found wedge)
//! Will enlarge the feature vectors of all vertices to size 22
//! Data will be written at Feature Vector Position 22 if vertices lie on a border between two clusterings
bool experimentalComputeRANSAC(	int numberOfIterations,
								string outputFileName,
								vector<vector<double>> &meshIntrinsicExtractedTetraeders,
								bool useNMSResultsForTetraederTop,
								double minimumTetraederHeight,
								bool extendMesh,
								bool addSeparationWall,
								bool visualizeRANSACQuality,
								bool visualizeTetraederHeight,
								vector<Vertex*> &mVertices,
								vector<Face*> &mFaces){

	//will note down the number of different labels used
	//is likely the same a the number of maxima that non maximum suppression produced
	int numberOfLabels = 0;

	int verticesWithoutLabel = 0;
	int verticesWithoutAssignedCluster = 0;

	//The extracted Tetraeders are collected and then written to a file
	vector<vector<Vertex*>> extractedTetraeders;

	vector<double> RANSACQualities;
	vector<double> TetraederHeights;

	map<int, Vertex*> localMaximaAccessedbyLabel;

	//prepare data
	//find the number of labels and the number of vertices, which were not labeled
	for(auto pVertex : mVertices){

		//if the feature vector has less than 22 elements, resize it to length 22
		if(22 > pVertex->getFeatureVectorLen()){

			pVertex->resizeFeatureVector(22);

		}

		//clear data, that might sit in the feature vectors, remaining from previous runs of the RANSAC algorithm
		pVertex->setFeatureElement(21, 0.0);

		double isLocalMaximum;
		pVertex->getFeatureElement(16, &isLocalMaximum);

		double givenLabel;
		pVertex->getFeatureElement(19, &givenLabel);

		double assignedCluster;
		pVertex->getFeatureElement(20, &assignedCluster);



		//watershed labels start at 1
		if(givenLabel < 0.5){

			//a vertex was not labeled in the watershed step
			verticesWithoutLabel++;

		}else{ //find out how many labels are used

			if((int)round(givenLabel) > numberOfLabels){

				numberOfLabels = givenLabel;

			}

			//and note down if it is a local maximum

			if(isLocalMaximum > 0.5){ //localMaxima are given a 1.0
				int currentLabel = (int)round(givenLabel);
				localMaximaAccessedbyLabel.insert( pair<int, Vertex*>(currentLabel,pVertex));

			}
		}

		//Assigned Clusters are either 1.0, 2.0 or 3.0
		//If no Cluster was assigned, then it says at Position 21 (Index 20) value 0.0
		if(assignedCluster < 0.5){
			// a vertex was not assigned to a cluster in the clustering step
			verticesWithoutAssignedCluster++;

		}
	}

	//number of clusters is now known

	//loop over every labeled group
	for(int labelIterator=1;labelIterator<=numberOfLabels;labelIterator++){

		//vector<Vertex*> verticesWithCurrentLabel;

		//may be used for more elaborate choice of vertices
		//currently can be used to find the tetraeder outer points
		//vector<Vertex*> verticesBorderGroup1And2;
		//vector<Vertex*> verticesBorderGroup2And3;
		//vector<Vertex*> verticesBorderGroup3And1;

		vector<Vertex*> verticesBorderGroup;

		//add vertices of the currently investigated label to a big vector
		for(auto currentVertex : mVertices){

			double foundLabel;
			currentVertex->getFeatureElement(19, &foundLabel);

			if(labelIterator == (int)round(foundLabel)){

				//so a vertex was found to be part of the current label group

				//verticesWithCurrentLabel.push_back(currentVertex);

				//collect all vertices that have a neighbour with a different label

				double currentVertexClusterGroup;
				currentVertex->getFeatureElement(20, &currentVertexClusterGroup);

				list<Vertex*> adjacentVertsInOrder;
				getSurroundingVerticesInOrder(adjacentVertsInOrder, currentVertex, false);

				bool borderVertexFound = false;

				//bool borderVertexWasAddedToDistinctBorderGroup = false;

				for(auto& adjacentVertex : adjacentVertsInOrder){

					//to abort as soon as a borderVertex is found
					if(!borderVertexFound){

						//only consider neighbours in the same label group
						//this is important because on the border of a labeled group neighbours may belong to different clusters,
						//but we only care about the current label group
						double adjacentVertexLabel;
						adjacentVertex->getFeatureElement(19, &adjacentVertexLabel);

						if(labelIterator == (int)round(adjacentVertexLabel)){

							//so the neighbour belongs to the same label group

							double adjacentVertexClusterGroup;
							adjacentVertex->getFeatureElement(20, &adjacentVertexClusterGroup);

							if(round(currentVertexClusterGroup) != round(adjacentVertexClusterGroup)){

								//the current Vertex was found to be a border vertex

								borderVertexFound = true;

								//find out on what border the vertex lies

								//border case 1 and 2 (or 2 and 1)
								if(			((round(currentVertexClusterGroup) == 1) && (round(adjacentVertexClusterGroup) == 2))
									||		((round(currentVertexClusterGroup) == 2) && (round(adjacentVertexClusterGroup) == 1)) ){

									currentVertex->setFeatureElement(21, 12.0);
									//currentVertex->setFeatureElement(22, 2.0);

								//border case 2 and 3 (or 3 and 2)
								} else if (	((round(currentVertexClusterGroup) == 2) && (round(adjacentVertexClusterGroup) == 3))
									||		((round(currentVertexClusterGroup) == 3) && (round(adjacentVertexClusterGroup) == 2)) ){

									currentVertex->setFeatureElement(21, 23.0);
									//currentVertex->setFeatureElement(23, 3.0);

								//border case 1 and 3 (or 3 and 1)
								} else if (	((round(currentVertexClusterGroup) == 3) && (round(adjacentVertexClusterGroup) == 1))
									||		((round(currentVertexClusterGroup) == 1) && (round(adjacentVertexClusterGroup) == 3)) ){

									currentVertex->setFeatureElement(21, 31.0);
									//currentVertex->setFeatureElement(23, 3.0);
								}
							}
						}
					}

				}

				//if the current vertex belongs to the border group add it to the vector verticesBordergroup
				if(borderVertexFound){

					verticesBorderGroup.push_back(currentVertex);

				}

			}
		}
		//The vector verticesBorderGroup now holds all the vertices which will be used for RANSAC

		//the sum of distances represents the quality of the chosen points and their corresponding rays
		//is initialized with an infinity placeholder
		double smallestSumOfDistances = numeric_limits<double>::max();

		//unassigned pointers are dangerous
		//They are filled 30 lines down from here
		//could be filled with dummy data
		Vertex* finalTetraederTop;
		Vertex* finalRayVertex1;
		Vertex* finalRayVertex2;
		Vertex* finalRayVertex3;




		//only work on a border group if it includes at least 4 vertices. This is a necessary requirement, as RANSAC needs 4 randomly chosen vertices to start
		//If the variant of the algorithm is chosen, where 3 vertices are randomly chosen, still a fourth is needed to be potentially identified as tetraeder top
		if(verticesBorderGroup.size()>=4){

			if(useNMSResultsForTetraederTop){//so the local maximum should be used as the tetraeder Top
				//using the local maximum as tetraeder top is NOT only possible if it is part of the vertices border group.
				//But the three randomly chosen vertices should not contain the local maximum
				Vertex* currentMaximum = localMaximaAccessedbyLabel[labelIterator];


				vector<Vertex*>::iterator it = find(verticesBorderGroup.begin(), verticesBorderGroup.end(), currentMaximum);
				if(it != verticesBorderGroup.end()){
					//so the local maximum is part of the border group
					//erase it, so it is not randomly chosen
					verticesBorderGroup.erase(it);
				}
				//having erased the tetraeder top from the border vertices should make no difference in distance calculation
				//because the tetraedertop has all 3 rays go through it. Those distances should all be zero.

				//RANSAC begins
				for(int iteration=0;iteration<numberOfIterations;iteration++){

					double currentDistanceForThisChoice = 0.0;

					vector<Vertex*> threeRandomlyChosenBorderVertices;
					wERandomlyChooseVerticesFromVector(verticesBorderGroup, threeRandomlyChosenBorderVertices, 3);

					Vertex* chosenTetraederTop = currentMaximum; //this vertex will remain the same in all RANSAC runs
					Vertex* randomlyChosenTetraederVertex1 = threeRandomlyChosenBorderVertices[0];
					Vertex* randomlyChosenTetraederVertex2 = threeRandomlyChosenBorderVertices[1];
					Vertex* randomlyChosenTetraederVertex3 = threeRandomlyChosenBorderVertices[2];

					for(auto borderVertex : verticesBorderGroup){

						//find minimum distance to one of the rays
						double minimumDistance = min(min(wEComputeShortestDistanceBetweenPointAndRay(borderVertex, chosenTetraederTop, randomlyChosenTetraederVertex1),
													wEComputeShortestDistanceBetweenPointAndRay(borderVertex, chosenTetraederTop, randomlyChosenTetraederVertex2)),
													wEComputeShortestDistanceBetweenPointAndRay(borderVertex, chosenTetraederTop, randomlyChosenTetraederVertex3));
						currentDistanceForThisChoice += minimumDistance;
					}

					if(currentDistanceForThisChoice < smallestSumOfDistances){

						smallestSumOfDistances = currentDistanceForThisChoice;

						finalTetraederTop = chosenTetraederTop;
						finalRayVertex1 = randomlyChosenTetraederVertex1;
						finalRayVertex2 = randomlyChosenTetraederVertex2;
						finalRayVertex3 = randomlyChosenTetraederVertex3;
					}

				}

			} else { //4 vertices will be chosen randomly, the local maximum will not be automatically used as tetraeder top.

				//RANSAC begins
				for(int iteration=0;iteration<numberOfIterations;iteration++){

					double currentDistanceForThisChoice = 0.0;

					//randomly choose four vertices for RANSAC
					//This includes possibly choosing the local maximum
					vector<Vertex*> fourRandomlyChosenBorderVertices;
					wERandomlyChooseVerticesFromVector(verticesBorderGroup, fourRandomlyChosenBorderVertices, 4);

					Vertex* randomlyChosenTetraederTop = fourRandomlyChosenBorderVertices[0];
					Vertex* randomlyChosenTetraederVertex1 = fourRandomlyChosenBorderVertices[1];
					Vertex* randomlyChosenTetraederVertex2 = fourRandomlyChosenBorderVertices[2];
					Vertex* randomlyChosenTetraederVertex3 = fourRandomlyChosenBorderVertices[3];

					for(auto borderVertex : verticesBorderGroup){

						//find minimum distance to one of the rays
						double minimumDistance = min(min(wEComputeShortestDistanceBetweenPointAndRay(borderVertex, randomlyChosenTetraederTop, randomlyChosenTetraederVertex1),
													wEComputeShortestDistanceBetweenPointAndRay(borderVertex, randomlyChosenTetraederTop, randomlyChosenTetraederVertex2)),
													wEComputeShortestDistanceBetweenPointAndRay(borderVertex, randomlyChosenTetraederTop, randomlyChosenTetraederVertex3));
						currentDistanceForThisChoice += minimumDistance;
					}

					if(currentDistanceForThisChoice < smallestSumOfDistances){

						smallestSumOfDistances = currentDistanceForThisChoice;

						finalTetraederTop = randomlyChosenTetraederTop;
						finalRayVertex1 = randomlyChosenTetraederVertex1;
						finalRayVertex2 = randomlyChosenTetraederVertex2;
						finalRayVertex3 = randomlyChosenTetraederVertex3;

					}
				}
			}
			//RANSAC now has noted down the current best fit for rays.

			//For each Ray, find the vertex close to it with the farthest away projection point
			vector<Vertex*> closestToRay1;
			vector<Vertex*> closestToRay2;
			vector<Vertex*> closestToRay3;

			for(auto borderVertex : verticesBorderGroup){
				if(		(wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex1) <
						wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex2))				&&
						(wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex1) <
						wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex3))				){
					closestToRay1.push_back(borderVertex);
				} else if	((wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex2) <
							wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex1))			&&
							(wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex2) <
							wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex3))			) {
					closestToRay2.push_back(borderVertex);
				} else if	((wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex3) <
							wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex1))			&&
							(wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex3) <
							wEComputeShortestDistanceBetweenPointAndRay(borderVertex, finalTetraederTop, finalRayVertex2))			) {
					closestToRay3.push_back(borderVertex);
				}
			}


			double greatestDistanceRay1 = 0.0;
			double greatestDistanceRay2 = 0.0;
			double greatestDistanceRay3 = 0.0;
			double currentDistanceRay1;
			double currentDistanceRay2;
			double currentDistanceRay3;

			Vertex* finalTetraederVertex1;
			Vertex* finalTetraederVertex2;
			Vertex* finalTetraederVertex3;

			for(auto vertexClosestToRay1 : closestToRay1){

				wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(vertexClosestToRay1, finalTetraederTop, finalRayVertex1, currentDistanceRay1);
				if(currentDistanceRay1 > greatestDistanceRay1){
					finalTetraederVertex1 = vertexClosestToRay1;
					greatestDistanceRay1 = currentDistanceRay1;
				}
			}

			for(auto vertexClosestToRay2 : closestToRay2){

				wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(vertexClosestToRay2, finalTetraederTop, finalRayVertex2, currentDistanceRay2);
				if(currentDistanceRay2 > greatestDistanceRay2){
					finalTetraederVertex2 = vertexClosestToRay2;
					greatestDistanceRay2 = currentDistanceRay2;
				}
			}

			for(auto vertexClosestToRay3 : closestToRay3){

				wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(vertexClosestToRay3, finalTetraederTop, finalRayVertex3, currentDistanceRay3);
				if(currentDistanceRay3 > greatestDistanceRay3){
					finalTetraederVertex3 = vertexClosestToRay3;
					greatestDistanceRay3 = currentDistanceRay3;
				}
			}
			//the three vertices finalTetraederVertices now hold the tetraeder




			//the following is legacy code

			//possible bordergroups
			/*
			12
			23
			31
			*/

			/*
			int borderGroupFinalTetVertex1;
			wEGetBorderGroupFromVertexByFeatureVector(finalTetraederVertex1, borderGroupFinalTetVertex1);

			int borderGroupFinalTetVertex2;
			wEGetBorderGroupFromVertexByFeatureVector(finalTetraederVertex2, borderGroupFinalTetVertex2);

			int borderGroupFinalTetVertex3;
			wEGetBorderGroupFromVertexByFeatureVector(finalTetraederVertex3, borderGroupFinalTetVertex3);
			*/

			//determine to which bordergoup which finalTetraederVertex belongs

			/*
			Vertex* finalTetraederVertexGroup12;
			Vertex* finalTetraederVertexGroup23;
			Vertex* finalTetraederVertexGroup31;

			wEGetBorderGroupFromVertexByFeatureVector(finalLineVertex1, finalLineVertex2, finalLineVertex3, finalTetraederVertexGroup12, finalTetraederVertexGroup23, finalTetraederVertexGroup31);


			//Determine which projected point on each line is farthest away
			//A faster but less precise way is to simply calculate the distance between the tetraeder top and the vertex itself and NOT the projected point on the line

			double biggestDistanceGroup12 = 0.0;
			Vertex* finalVertexGroup12;

			double biggestDistanceGroup23 = 0.0;
			Vertex* finalVertexGroup23;

			double biggestDistanceGroup31 = 0.0;
			Vertex* finalVertexGroup31;

			for(auto borderVertex : verticesBorderGroup){

				//int currentborderVertexGroup;
				//wEGetBorderGroupFromVertexByFeatureVector(borderVertex, cuhttps://stackoverflow.com/questions/40454789/computing-face-normals-and-windingrrentborderVertexGroup);

				double currentBorderVertexGroup;
				borderVertex->getFeatureElement(21, &currentBorderVertexGroup);

				//keep of the biggest distance per border group
				//note down the vertex that has the projection farthest away

				if(round(currentBorderVertexGroup) == 12){

					double calculatedDistance;

					wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(borderVertex, finalTetraederTop, finalVertexGroup12 , calculatedDistance);
					if(calculatedDistance>biggestDistanceGroup12){

						biggestDistanceGroup12 = calculatedDistance;
						finalVertexGroup12 = borderVertex;

					}

				} else if (round(currentBorderVertexGroup) == 23){

					double calculatedDistance;

					wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(borderVertex, finalTetraederTop, finalVertexGroup23 , calculatedDistance);
					if(calculatedDistance>biggestDistanceGroup23){

						biggestDistanceGroup23 = calculatedDistance;
						finalVertexGroup23 = borderVertex;

					}

				} else if (round(currentBorderVertexGroup) == 31){

					double calculatedDistance;

					wEComputeSquaredDistanceFromTetraederTopToProjectedPointOnLine(borderVertex, finalTetraederTop, finalVertexGroup31 , calculatedDistance);
					if(calculatedDistance>biggestDistanceGroup31){

						biggestDistanceGroup31 = calculatedDistance;
						finalVertexGroup31 = borderVertex;

					}

				}
			}*/


			/*

			A Tetraeder is:
			finalTetraederTop
			finalVertexGroup12
			finalVertexGroup23
			finalVertexGroup31

			*/

			//minimumTetraederHeight is not used right now, it is set externally

			vector<Vertex*> foundTetraeder= {finalTetraederTop, finalTetraederVertex1, finalTetraederVertex2, finalTetraederVertex3};

			//double allowedMinimumHeight = temporaryRANSACParameter;

			//use the temporary RANSAC PARAMETER, a temporary global, to only allow tetraeders with sufficient height
			/*
			if(wECheckTetraederHeight(foundTetraeder, allowedMinimumHeight)){
				extractedTetraeders.push_back(foundTetraeder);
			}
			*/

			//24.07. don't omit any tetraeders right now


			//collect the extracted tetraeder
			extractedTetraeders.push_back(foundTetraeder);

			//collect the extracted tetraeders RANSAC quality
			RANSACQualities.push_back(smallestSumOfDistances);

			//collect the extracted tetraeders height
			TetraederHeights.push_back(wEComputeTetraederHeight(foundTetraeder));




		}
		//One labeled group has been worked upon
		//The labeled group was only worked upon if it consisted of at least 4 vertices

	}//all labeled groups have been worked upon

	//Export found Wedges into .obj-file
	wEWriteExtractedTetraedersIntoFile(extractedTetraeders, outputFileName);

	//Export found Wedges into the mesh class for extraction
	//A python script can be used to write the wedge information into a csv-file
	wEWriteExtractedTetraedersIntoMeshForCollection(meshIntrinsicExtractedTetraeders, extractedTetraeders, RANSACQualities, TetraederHeights);


	//Export found Wedges into .ply-file TOGETHER with the original mesh
	//mVertices and mFaces are used to extract the original mesh

	//for visualization purposes, the RANSAC Quality and the Tetraeder Height are normalized
	vector<double> normalizedRANSACQualities;
	wENormalizeValues(RANSACQualities, normalizedRANSACQualities);

	vector<double> normalizedTetraederHeights;
	wENormalizeValues(TetraederHeights, normalizedTetraederHeights);

	//Parameters: the 1 is the mode for RANSAC Quality visualization and 2 ist the mode for tetraeder height visualization.
	wEWriteExtractedTetraedersAndMeshIntoFile(extractedTetraeders, outputFileName, normalizedRANSACQualities, 1, mVertices, mFaces);
	wEWriteExtractedTetraedersAndMeshIntoFile(extractedTetraeders, outputFileName, normalizedTetraederHeights, 2, mVertices, mFaces);
	/*
	if(visualizeRANSACQuality){
		vector<double> normalizedRANSACQualities;
		wENormalizeValues(RANSACQualities, normalizedRANSACQualities);
		wEWriteExtractedTetraedersAndMeshIntoFile(extractedTetraeders, outputFileName, normalizedRANSACQualities, mVertices, mFaces);
	} else if(visualizeTetraederHeight) {
		vector<double> normalizedTetraederHeights;
		wENormalizeValues(TetraederHeights, normalizedTetraederHeights);
		wEWriteExtractedTetraedersAndMeshIntoFile(extractedTetraeders, outputFileName, normalizedTetraederHeights, mVertices, mFaces);
	} else {
		//in case the user does not want a wedge property visualized, wedges are shown in the same tone of green
		vector<double> emptyVector;
		wEWriteExtractedTetraedersAndMeshIntoFile(extractedTetraeders, outputFileName, emptyVector, mVertices, mFaces);
	}*/


	cout << "RANSAC terminated successfully!" << endl;

	//obsolete output, as with improved watershed, some vertices will not be reached
	//cout << "RANSAC encountered " << verticesWithoutAssignedCluster << " vertices that were not assigned to any cluster." << endl;


	return true;

}


bool experimentalReorderFeatureVector(	vector<Vertex*> &mVertices,
										vector<Face*> &mFaces,
										double deletableInput){

	temporaryNMSParameter = deletableInput;
	temporaryWaterParameter = deletableInput;

	cout << deletableInput << " as Value was given" << endl;

	return true;

}
