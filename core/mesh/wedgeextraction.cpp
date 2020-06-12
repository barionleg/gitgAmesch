
/*
Written by Martin Seiler
Part of the Master Thesis

Status: Experimental

Please look at mesh.cpp for larger and more complete disclaimer by original creators

*/

#include<iostream>
#include<vector>
#include<iterator>
#include<queue>

#include<algorithm>
#include<random>
#include<chrono>
#include<math.h> //for sqrt, can hopefully be omitted

#include<limits> //used for infinity placeholders in RANSAC

#include <GigaMesh/mesh/wedgeextraction.h>

#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/mesh.h>

using namespace std;

//helper methods exclusively called in the wedge extraction project
//they all begin with wE

//! distance calculation omitting square root for speed up
double squaredDistanceBetweenTwoVertices(Vertex* &vertexNo1, Vertex* &vertexNo2){
/*
	return(	(vertexNo1->getX() - vertexNo2->getX()) * (vertexNo1->getX() - vertexNo2->getX())	+
			(vertexNo1->getY() - vertexNo2->getY()) * (vertexNo1->getY() - vertexNo2->getY())	+
			(vertexNo1->getZ() - vertexNo2->getZ()) * (vertexNo1->getZ() - vertexNo2->getZ())	);
*/

	return(	pow((vertexNr1->getX() - vertexNr2->getX()), 2.0)+
			pow((vertexNr1->getY() - vertexNr2->getY()), 2.0)+
			pow((vertexNr1->getZ() - vertexNr2->getZ()), 2.0)	);

}

//! distance calculation for normals omitting square root for speed up
double squaredDistanceBetweenTwoNormalsTreatedAsVertices(Vertex* &vertexNo1, Vertex* &vertexNo2){
/*
	return(	(vertexNo1->getX() - vertexNo2->getX()) * (vertexNo1->getX() - vertexNo2->getX())	+
			(vertexNo1->getY() - vertexNo2->getY()) * (vertexNo1->getY() - vertexNo2->getY())	+
			(vertexNo1->getZ() - vertexNo2->getZ()) * (vertexNo1->getZ() - vertexNo2->getZ())	);
*/

	return(	pow((vertexNr1->getNormalX() - vertexNr2->getNormalX()), 2.0)+
			pow((vertexNr1->getNormalY() - vertexNr2->getNormalY()), 2.0)+
			pow((vertexNr1->getNormalZ() - vertexNr2->getNormalZ()), 2.0)	);

}

//!
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


//! currently unused
void wEFindMedianInListForGivenMean(list<Vertex*> &inputList, double &meanNormalX, double &meanNormalY, double &meanNormalZ, double &medianNormalX, double &medianNormalX, double &medianNormalX){

    //check back with supervisor

}


//! assign
void wEAssignNormalsToNearestMean(vector<double> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<Vertex*> &threeVerticesWhoseNormalsAreUsed){

}

//! compute
void wEComputeMean(vector<double> &currentClustering, vector<Vertex*> &verticesWithCurrentLabel, vector<Vertex*> &threeVerticesWhoseNormalsAreUsed){

}

//! computes the shortest distance between a point (arbPoint) and a line defined by two points lying on the line
//! TODO could be rewritten using a class or other helper methods
//! currently uses the square root, which could be omitted I think
double wEComputeShortestDistanceBetweenPointAndLine(Vertex* &arbPoint, Vertex* &point1OnLine, Vertex* &point2OnLine){

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

	return parallelogramArea / lineSegmentLength;

}

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

void wEDistanceFromTetraederTopToProjectedPointOnLine(Vertex* &arbPoint, Vertex* &point1OnLineTetraederTop, Vertex* &point2OnLine, double &computedDistance){

	double vector1ComponentX = arbPoint->getX() - point1OnLineTetraederTop->getX();
	double vector1ComponentY = arbPoint->getY() - point1OnLineTetraederTop->getY();
	double vector1ComponentZ = arbPoint->getZ() - point1OnLineTetraederTop->getZ();

	double vector2ComponentX = point2OnLine->getX() - point1OnLineTetraederTop->getX();
	double vector2ComponentY = point2OnLine->getY() - point1OnLineTetraederTop->getY();
	double vector2ComponentZ = point2OnLine->getZ() - point1OnLineTetraederTop->getZ();



	double dotProduct1 = vector1ComponentX * vector2ComponentX + vector1ComponentY * vector2ComponentY + vector1ComponentZ * vector2ComponentZ;
	double dotProduct2 = vector1ComponentX * vector1ComponentX + vector1ComponentY * vector1ComponentY + vector1ComponentZ * vector1ComponentZ;

	double projectedPointOnLineComponentX = point1OnLineTetraederTop->getX() + dotProduct1/dotProduct2 * vector2ComponentX;
	double projectedPointOnLineComponentY = point1OnLineTetraederTop->getY() + dotProduct1/dotProduct2 * vector2ComponentY;
	double projectedPointOnLineComponentZ = point1OnLineTetraederTop->getZ() + dotProduct1/dotProduct2 * vector2ComponentZ;

	//TODO continue computation of distance to TetraederTop

}




//methods called in mesh.cpp


//! an experimental non maximum suppression method.
//! Takes as input the distance, how far NMS should look (the frontier will progress until vertices are too far away)
//! will enlarge the feature vectors of all vertices to size 18
//! Data will be written at Feature Vector Position 17 (boolean if maximum) and 18 (if maximum, then averaged MSII)
bool experimentalNonMaximumSuppression(double &NMSDistance, vector<Vertex*> &mVertices, uint64_t &numberOfVertices){

	//these counters are used for feedback in the terminal
	int processedVertexCounter = 0;
	int maximaCounter = 0;

	//look up vector, will be cleared for every vertex.
	vector<bool> checkVisited (numberOfVertices);

	//avoid using square root to speed things u
	double squaredDistance = NMSDistance * NMSDistance;

	//loop over all vertices
	for(auto pVertex : mVertices){

		//initialize the checks for visited vertices with false
		//maybe use methods that already exist in vertex.cpp
		for (int i = 0; i < checkVisited.size() ; i++){
			checkVisited[i] = false;
			//maybe this works too: checkVisited.at(i) = false
		}

		//if the feature vector has less than 18 elements, resize it to length 18
		//the current concept is: use the feature vector and write computated values at position 17 and 18
		//resizing pads with zeros
		if(18 > pVertex->getFeatureVectorLen()){
			pVertex->resizeFeatureVector(18);
		}

		//used for relative feature vector element access
		unsigned int currentFeatureVectorLength = pVertex->getFeatureVectorLen();

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
		//however I like i more
		pVertex->getFeatureElement(15, &MSIIcomponent16);

		//Vertices, which will be the input for every search
		queue<Vertex*> marchingFrontier;

		marchingFrontier.push(pVertex);

		//if the list marchingFrontier is empty, there are no vertices to be checked.

		//as long as there are eligible candidates, perform the algorithm
		//only perform the algorithm when no bigger MSII value than the one from the current vertex (pVertex) has been found
		while(!marchingFrontier.empty() && !biggerValueFound){

			list<Vertex*> adjacentVertsInOrder;
			getSurroundingVerticesInOrder(adjacentVertsInOrder, marchingFrontier.front(), false);

			//vertices are marked as visited, when they they are asked for their neighbours
			checkVisited[marchingFrontier.front()->getIndexOriginal()] = true;

			//after the front vertex of the queue was marked as visitied, it is popped
			marchingFrontier.pop();

			//loop over the new found neighbours
			for(auto& adjacentVertex : adjacentVertsInOrder){

				//only perform the algorithm, if no bigger value was found AND the candidate has not been visited yet
				if(!biggerValueFound && !(checkVisited[adjacentVertex->getIndexOriginal()])){

					bool stillInReach = false;

					//helper method for distance calculation is used
					if( squaredDistanceBetweenTwoVertices(adjacentVertex, pVertex) <= squaredDistance){

						stillInReach = true;

					}

					//consider only vertices that are within the reach specified by the user
					if(stillInReach){

						double adjacentVertexsMSIIcomponent16;
						//adjacentVertex->getFeatureElement( (currentFeatureVectorLength-3) , &adjacentVertexsMSIIcomponent16);

						//legacy, position access deemed too absolute
						//however I like it more
						candidatesIterator->getFeatureElement(15, &adjacentVertexsMSIIcomponent16);

						// if a bigger MSII value than the one of the current vertex (pVertex) was found, pVertex is not the (lokal) maximum
						if(MSIIcomponent16 <= adjacentVertexsMSIIcomponent16){

							biggerValueFound = true;
							isMax = false; //not necessary

						}else{ //so it still seems to be the (local) maximum

							isMax = true; //the current Vertex temporarily seems to be a maximum

							//the new found neighbour doesn't have a bigger MSII value, so it is added to the front (put at the end of the queue
							//maybe his neighbours have a bigger MSII value
							marchingFrontier.push(adjacentVertex);

						}
					}
				}
			}
		}
		//if after the while loop the boolean isMax was set to true and remained true, the current vertex pVertex is a maximum

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

		if((processedVertexCounter % 10000) == 0){

			cout << processedVertexCounter << " of " << numberOfVertices << " vertices have been processed until now." << endl;

		}

	}// all vertices were looped over

	cout << "Given the radius of: " << NMSDistance << " these many MSII values (local maxima) survived non maximum suppression: " << maximaCounter << endl;

	return true;

}


//! An experimental Watershed method using values computed in the Non Maximum Suppression Method
//! will enlarge the feature vectors of all vertices to size 20
//! Data will be written at Feature Vector Position 19 (order of processing) and 20 (label)
bool experimentalWaterShed(vector<Vertex*> &mVertices, uint64_t &numberOfVertices){

	double counterForFutureWork = 1.0;

	//prepare a look up vector to mark visited vertices
	vector<bool> checkVisited (numberOfVertices);

	//initialize the checks for visited vertices with false
	for (int i = 0; i < checkVisited.size() ; i++){
		checkVisited[i] = false;
		//maybe this works too: checkVisited.at(i) = false
	}

	//list to retrieve all vertices which are maxima
	//they have 1.0 at feature vector position 17 (index 16)
	list<Vertex*> foundMaxima;

	//labels start with 1 and go up to (the number of maxima)
	double experimentalLabel = 1.0;

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

			pVertex->setFeatureElement(18, counterForFutureWork);
			pVertex->setFeatureElement(19, experimentalLabel);

			int vertexIndex = pVertex->getIndexOriginal();
			checkVisited[vertexIndex] = true;

			counterForFutureWork = counterForFutureWork + 1.0;
			experimentalLabel = experimentalLabel + 1.0;

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

			int vertexIndex = coastCandidate->getIndexOriginal();

			//only continue, if the vertex was not visited yet
			if(!checkVisited[vertexIndex]){

				double currentCandidateLabel;
				coastCandidate->getFeatureElement(19, &currentCandidateLabel);

				//label the vertex, if it has no label yet
				//labels start at 1.0, a value below means no label was given yet
				if(currentCandidateLabel < 1.0){

					coastCandidate->setFeatureElement(19, maximumLabel);

					counterForFutureWork = counterForFutureWork + 1.0;

					//add the vertex to the coast
					//the coast is sorted by the MSII-Value

					double coastCandidateMSIIValue;
					coastCandidate->getFeatureElement(15, &coastCandidateMSIIValue);

					//there might be a loss of vertices, if vertices have the same MSII value (at position 16 (index 15)) because the map does not allow multiple equal keys
					watershedCoast.insert( pair<double, Vertex*>(coastCandidateMSIIValue, coastCandidate) );

					//DO NOT mark the vertex as visited
					//A Vertex gets marked as visited ONLY, when its neighbours have been retrieved

				}
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

			int candidateIndex = newCoastCandidate->getIndexOriginal();

			//only continue, if the vertex was not visited yet
			if(!checkVisited[candidateIndex]){

				double currentCandidateLabel;
				newCoastCandidate->getFeatureElement(19, &currentCandidateLabel);

				//label the vertex, if it has no label yet
				//labels start at 1.0, a value below means no label was given yet
				if(currentCandidateLabel < 1.0){

					newCoastCandidate->setFeatureElement(19, maximumLabel);

				}

				//add the vertex to the coast
				//the coast is sorted by the MSII-Value

				double coastVertexMSIIValue;
				newCoastCandidate->getFeatureElement(15, &coastVertexMSIIValue);

				//there might be a loss of vertices, if vertices have the same MSII value (at position 16 (index 15)) because the map does not allow multiple equal keys
				watershedCoast.insert( pair<double, Vertex*>(coastVertexMSIIValue, newCoastCandidate) );

			}
		}

		//the current vertex with the largest MSII value (maybe now not anymore) has been searched for neighbours
		//so mark it as visited and note down when it was treated

		VertexPWithLargestMSIIValue->setFeatureElement(18, counterForFutureWork);
		counterForFutureWork = counterForFutureWork + 1.0;

		//mark the Vertex as visited (after its neighbours were added to the coast)
		int vertexIndex = VertexPWithLargestMSIIValue->getIndexOriginal();
		checkVisited[vertexIndex] = true;

		//remove the vertex from the coast
		//DO NOT // watershedCoast.erase(prev(watershedCoast.end())); //THIS IS WRONG, meanwhile the highest element no longer has to be the vertex we looked at, because we inserted other vertices
		watershedCoast.erase(VPWMSIIValue);

		if(((int)counterForFutureWork % 10000) == 0){

			cout << counterForFutureWork << " of " << numberOfVertices << " vertices have been processed." << endl;
		}
	}

	//consider implementation using multimap
	//to avoid the following

	//postprocessing the map solution


	//the following works as long as no unlabeled vertex is surrounded by unlabeled vertices

	int notLabeledYet = 0;
	int postProcessingCouldLabel = 0;

	//find all vertices that don't have a label yet and give them a label of an arbitrary neighbour
	for(auto pVertex : mVertices){

		double checkLabel;
		pVertex->getFeatureElement(19, &checkLabel);

		//no label given
		if(checkLabel < 1.0){

			//note down that it happened
			notLabeledYet++;
			list<Vertex*> adjacentVertsInOrder;
			getSurroundingVerticesInOrder(adjacentVertsInOrder, pVertex, false);

			bool labelCouldBeAssigned = false;

			//use neighbours to assign a label arbitrarily
			for(auto& neighbourVertex : adjacentVertsInOrder){

				if(!labelCouldBeAssigned){

					double possibleLabel;
					neighbourVertex->getFeatureElement(19, &possibleLabel);

					//if the neighbouring vertex has a label, assign it to pVertex
					if(possibleLabel > 0.0){

						pVertex->setFeatureElement(19, possibleLabel);

						postProcessingCouldLabel++;

						labelCouldBeAssigned = true;

					}
				}
			}
		}
	}

	cout << foundMaxima.size() << " maxima formed the base for the watershed algorithm." << endl;

	cout << "The watershed algorithm did not reach " << notLabeledYet << " vertices." << endl;
	cout << "Postprocessing gave " << postProcessingCouldLabel << " of these vertices a label." << endl;

	return true;

}


//! An experimental clustering method
//! It does not use in-cluster variance to judge how good the clustering is
//! Instead it relies on multiple iterations and mean adjusting
//! will enlarge the feature vectors of all vertices to size 21
//! Data will be written at Feature Vector Position 21 (sub clustering)
bool experimentalClustering(int numberOfIterations, vector<Vertex*> &mVertices){

	//will note down the number of different labels used
	//is likely the same a the number of maxima that non maximum suppression produced
	double numberOfLabels = 0.0;

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

	//number of clusters is now known

	//loop over every cluster
	for(int labelIterator=1;labelIterator<=(int)numberOfLabels;labelIterator++){

		vector<Vertex*> verticesWithCurrentLabel;

		//add vertices of the currently investigated label to a big vector
		for(auto investigatedVertex : mVertices){

			double foundLabel;
			investigatedVertex->getFeatureElement(19, &foundLabel);

			//smelly, comparing doubles for equality, however I don't know a better way at the moment
			if((double)labelIterator == foundLabel){

				verticesWithCurrentLabel.push_back(investigatedVertex);

			}
		}
		//The vector verticesWithCurrentLabel now holds all the vertices which will be used for clustering
		//there already is some randomness involved in creating this vector
		//however another shuffling or random selection of 3 cluster means is done

		vector<Vertex*> threeRandomlyChosenVertices;
		//the random selection is done by a helper method
		wERandomlyChooseVerticesFromVector(verticesWithCurrentLabel, threeRandomlyChosenVertices, 3);

		vector<double> currentClustering (verticesWithCurrentLabel.size());

		//the assignment to the nearest mean is done by a helper method
		wEAssignNormalsToNearestMean(currentClustering, verticesWithCurrentLabel, threeRandomlyChosenVertices);

		//the initial cluster centers
		//Vertex* clusterCNr1 = threeRandomlyChosenVertices[0];
		//Vertex* clusterCNr2 = threeRandomlyChosenVertices[1];
		//Vertex* clusterCNr3 = threeRandomlyChosenVertices[2];

		for(int iteration=0;iteration<numberOfIterations;iteration++){

			vector<Vertex*> normalMeans (verticesWithCurrentLabel.size());

			//the mean computation is done by a helper method
			wEComputeMean(currentClustering, verticesWithCurrentLabel, normalMeans);

			wEAssignNormalsToNearestMean(currentClustering, verticesWithCurrentLabel, normalMeans);

		}
















		cout << "finished" << endl;



		//the vector finalClustering holds the final
		//vector<double> finalClustering (verticesWithCurrentLabel.size());

		//vector<double> nonFinalClustering (verticesWithCurrentLabel.size());


		//compute clustering

		//recompute as often as the user wants

		vector<double> temporaryClustering (verticesWithCurrentLabel.size());


		vector<Vertex*> verticesCluster1;
		vector<Vertex*> verticesCluster2;
		vector<Vertex*> verticesCluster3;

		//perform the k-means assignment part on the vertices with the current label
		for(int vertextNr = 0; vertextNr<verticesWithCurrentLabel.size(); vertextNr++){


			Vertex* currentVertex = verticesWithCurrentLabel[vertextNr];

			//find the smallest distance
			//square root is omitted

			//normals are used for clustering

			//currently no check if normal exists

			//currently no check if normal has length 1, is assumed

			double distanceToMean1 = pow((currentVertex->getNormalX() - clusterCNr1->getNormalX()), 2.0) + pow((currentVertex->getNormalY() - clusterCNr1->getNormalY()), 2.0) + pow((currentVertex->getNormalZ() - clusterCNr1->getNormalZ()), 2.0);
			double distanceToMean2 = pow((currentVertex->getNormalX() - clusterCNr2->getNormalX()), 2.0) + pow((currentVertex->getNormalY() - clusterCNr2->getNormalY()), 2.0) + pow((currentVertex->getNormalZ() - clusterCNr2->getNormalZ()), 2.0);
			double distanceToMean3 = pow((currentVertex->getNormalX() - clusterCNr3->getNormalX()), 2.0) + pow((currentVertex->getNormalY() - clusterCNr3->getNormalY()), 2.0) + pow((currentVertex->getNormalZ() - clusterCNr3->getNormalZ()), 2.0);

			//no check for greater equal
			if((distanceToMean1 < distanceToMean2) && (distanceToMean1 < distanceToMean3)){

				temporaryClustering[vertextNr] = 1.0; //cluster center 1 is the closest
				verticesCluster1.push_back(currentVertex);

			} else if((distanceToMean2 < distanceToMean1) && (distanceToMean2 < distanceToMean3)){

				temporaryClustering[vertextNr] = 2.0; //cluster center 2 is the closest
				verticesCluster2.push_back(currentVertex);

			} else if((distanceToMean3 < distanceToMean1) && (distanceToMean3 < distanceToMean2)){

				temporaryClustering[vertextNr] = 3.0; //cluster center 3 is the closest
				verticesCluster3.push_back(currentVertex);

			}
		}

		//first calculate the means of the clusters
		//then calculate the variance of the clusters and decide according to the variance
		//is there a step intended after the mean calculation, for example reclustering?
		//yes recluster until no further change
		//this is not yet implemented

		//variance might need covariance matrix    int investigatedVertCounter = 0;


		//cluster 1
		//mean calculation
		double mean1XComponent = 0.0;
		double mean1YComponent = 0.0;
		double mean1ZComponent = 0.0;
		for(int i = 0; i<verticesCluster1.size(); i++){

			Vertex* currentVertex = verticesCluster1[i];
			mean1XComponent = mean1XComponent + currentVertex->getNormalX();
			mean1YComponent = mean1YComponent + currentVertex->getNormalY();
			mean1ZComponent = mean1ZComponent + currentVertex->getNormalZ();

		}
		mean1XComponent = mean1XComponent/verticesCluster1.size();
		mean1YComponent = mean1YComponent/verticesCluster1.size();
		mean1ZComponent = mean1ZComponent/verticesCluster1.size();

        //perform the clustering as often as the user wants
        for(int iteration=0;iteration<numberOfIterations;iteration++){



            vector<double> temporaryClustering (verticesWithCurrentLabel.size());



            Vertex* clusterCNr1 = verticesWithCurrentLabel[randomNr1];
            Vertex* clusterCNr2 = verticesWithCurrentLabel[randomNr2];
            Vertex* clusterCNr3 = verticesWithCurrentLabel[randomNr3];

            vector<Vertex*> verticesCluster1;
            vector<Vertex*> verticesCluster2;
            vector<Vertex*> verticesCluster3;

            //perform the k-means assignment part on the vertices with the current label
            for(int vertextNr = 0; vertextNr<verticesWithCurrentLabel.size(); vertextNr++){

                Vertex* currentVertex = verticesWithCurrentLabel[vertextNr];

                //find the smallest distance
                //square root is omitted

                //normals are used for clustering

                //currently no check if normal exists

                //currently no check if normal has length 1, is assumed

                double distanceToMean1 = pow((currentVertex->getNormalX() - clusterCNr1->getNormalX()), 2.0) + pow((currentVertex->getNormalY() - clusterCNr1->getNormalY()), 2.0) + pow((currentVertex->getNormalZ() - clusterCNr1->getNormalZ()), 2.0);
                double distanceToMean2 = pow((currentVertex->getNormalX() - clusterCNr2->getNormalX()), 2.0) + pow((currentVertex->getNormalY() - clusterCNr2->getNormalY()), 2.0) + pow((currentVertex->getNormalZ() - clusterCNr2->getNormalZ()), 2.0);
                double distanceToMean3 = pow((currentVertex->getNormalX() - clusterCNr3->getNormalX()), 2.0) + pow((currentVertex->getNormalY() - clusterCNr3->getNormalY()), 2.0) + pow((currentVertex->getNormalZ() - clusterCNr3->getNormalZ()), 2.0);

                //no check for greater equal
                if((distanceToMean1 < distanceToMean2) && (distanceToMean1 < distanceToMean3)){

                    temporaryClustering[vertextNr] = 1.0; //cluster center 1 is the closest
                    verticesCluster1.push_back(currentVertex);

                } else if((distanceToMean2 < distanceToMean1) && (distanceToMean2 < distanceToMean3)){

                    temporaryClustering[vertextNr] = 2.0; //cluster center 2 is the closest
                    verticesCluster2.push_back(currentVertex);

                } else if((distanceToMean3 < distanceToMean1) && (distanceToMean3 < distanceToMean2)){

                    temporaryClustering[vertextNr] = 3.0; //cluster center 3 is the closest
                    verticesCluster3.push_back(currentVertex);

                }
            }

            //first calculate the means of the clusters
            //then calculate the variance of the clusters and decide according to the variance
            //is there a step intended after the mean calculation, for example reclustering?
            //yes recluster until no further change
            //this is not yet implemented

            //variance might need covariance matrix    int investigatedVertCounter = 0;


            //cluster 1
            //mean calculation
            double mean1XComponent = 0.0;
            double mean1YComponent = 0.0;
            double mean1ZComponent = 0.0;
            for(int i = 0; i<verticesCluster1.size(); i++){

                Vertex* currentVertex = verticesCluster1[i];
                mean1XComponent = mean1XComponent + currentVertex->getNormalX();
                mean1YComponent = mean1YComponent + currentVertex->getNormalY();
                mean1ZComponent = mean1ZComponent + currentVertex->getNormalZ();

            }
            mean1XComponent = mean1XComponent/verticesCluster1.size();
            mean1YComponent = mean1YComponent/verticesCluster1.size();
            mean1ZComponent = mean1ZComponent/verticesCluster1.size();

            //variance calculation
            double variance1XComponent = 0.0;
            double variance1YComponent = 0.0;
            double variance1ZComponent = 0.0;
            for(int i = 0; i<verticesCluster1.size(); i++){

                Vertex* currentVertex = verticesCluster1[i];
                variance1XComponent = variance1XComponent + pow((currentVertex->getNormalX() - mean1XComponent), 2.0);
                variance1YComponent = variance1YComponent + pow((currentVertex->getNormalY() - mean1YComponent), 2.0);
                variance1ZComponent = variance1ZComponent + pow((currentVertex->getNormalZ() - mean1ZComponent), 2.0);

            }
            variance1XComponent = variance1XComponent/verticesCluster1.size();
            variance1YComponent = variance1YComponent/verticesCluster1.size();
            variance1ZComponent = variance1ZComponent/verticesCluster1.size();
            //end of cluster 1 consideration


            //cluster 2
            //mean calculation
            double mean2XComponent = 0.0;
            double mean2YComponent = 0.0;
            double mean2ZComponent = 0.0;
            for(int i = 0; i<verticesCluster2.size(); i++){

                Vertex* currentVertex = verticesCluster2[i];
                mean2XComponent = mean2XComponent + currentVertex->getNormalX();
                mean2YComponent = mean2YComponent + currentVertex->getNormalY();
                mean2ZComponent = mean2ZComponent + currentVertex->getNormalZ();

            }
            mean2XComponent = mean2XComponent/verticesCluster2.size();
            mean2YComponent = mean2YComponent/verticesCluster2.size();
            mean2ZComponent = mean2ZComponent/verticesCluster2.size();

            //variance calculation
            double variance2XComponent = 0.0;
            double variance2YComponent = 0.0;
            double variance2ZComponent = 0.0;
            for(int i = 0; i<verticesCluster2.size(); i++){

                Vertex* currentVertex = verticesCluster2[i];
                variance2XComponent = variance2XComponent + pow((currentVertex->getNormalX() - mean2XComponent), 2.0);
                variance2YComponent = variance2YComponent + pow((currentVertex->getNormalY() - mean2YComponent), 2.0);
                variance2ZComponent = variance2ZComponent + pow((currentVertex->getNormalZ() - mean2ZComponent), 2.0);

            }
            variance2XComponent = variance2XComponent/verticesCluster2.size();
            variance2YComponent = variance2YComponent/verticesCluster2.size();
            variance2ZComponent = variance2ZComponent/verticesCluster2.size();
            //end of cluster 2 consideration


            //cluster 3
            //mean calculation
            double mean3XComponent = 0.0;
            double mean3YComponent = 0.0;
            double mean3ZComponent = 0.0;
            for(int i = 0; i<verticesCluster3.size(); i++){

                Vertex* currentVertex = verticesCluster3[i];
                mean3XComponent = mean3XComponent + currentVertex->getNormalX();
                mean3YComponent = mean3YComponent + currentVertex->getNormalY();
                mean3ZComponent = mean3ZComponent + currentVertex->getNormalZ();

            }
            mean3XComponent = mean3XComponent/verticesCluster3.size();
            mean3YComponent = mean3YComponent/verticesCluster3.size();
            mean3ZComponent = mean3ZComponent/verticesCluster3.size();

            //variance calculation
            double variance3XComponent = 0.0;
            double variance3YComponent = 0.0;
            double variance3ZComponent = 0.0;
            for(int i = 0; i<verticesCluster3.size(); i++){

                Vertex* currentVertex = verticesCluster3[i];
                variance3XComponent = variance3XComponent + pow((currentVertex->getNormalX() - mean3XComponent), 2.0);
                variance3YComponent = variance3YComponent + pow((currentVertex->getNormalY() - mean3YComponent), 2.0);
                variance3ZComponent = variance3ZComponent + pow((currentVertex->getNormalZ() - mean3ZComponent), 2.0);

            }
            variance3XComponent = variance3XComponent/verticesCluster3.size();
            variance3YComponent = variance3YComponent/verticesCluster3.size();
            variance3ZComponent = variance3ZComponent/verticesCluster3.size();
            //end of cluster 3 consideration


            double totalVariance = variance1XComponent + variance1YComponent + variance1ZComponent + variance2XComponent + variance2YComponent + variance2ZComponent + variance3XComponent + variance3YComponent + variance3ZComponent;


            //a better clustering has been found save that clustering
            if(totalVariance < smallestVariance){

                finalClustering.assign(temporaryClustering.begin(), temporaryClustering.end());
                smallestVariance = totalVariance;

            }

        }
        //k-means has been performed as often as the user wanted


        //write the clustering into the feature vector
        for(int i = 0;i<verticesWithCurrentLabel.size();i++){

            Vertex* currentVertex = verticesWithCurrentLabel[i];
            currentVertex->setFeatureElement(20, finalClustering[i]);

        }
    }




    //loop over 1 till numberOfLabels
    //a list forms all the vertices with the current label
    //smallestVariance = infinite
    //get the list's size, build a vector of double finalClustering
    //FOR ITERATION NUMBER, deletableInput may become this value, for example 5 times
    //a good value needs to be determined by me, maybe 2 is good, maybe 10 is good

    //get the list's size, build a vector of doubles temporaryClustering

    //loop over that list
    //cluster

    //cluster is:
    //choose three at random (give them 1, 2, 3)
    //all other get assigned the clustervalue of the nearest

    //calculate variance
    //if the calculated variance is smaller than the smallestVariance
    //than write temporaryClustering in finalClustering
    //set smallestVariance to the calculated variance


    cout << "The clustering algorithm found " << verticesWithoutLabel << " vertices without label." << endl;
    cout << "The clustering algorithm worked on " << numberOfLabels  << " labels, k-means found 3 clusters in each labeled group." << endl;

    return true;

}

//! will enlarge the feature vectors of all vertices to size 24
bool experimentalRANSAC(int numberOfIterations, vector<Vertex*> &mVertices){

	//will note down the number of different labels used
	//is likely the same a the number of maxima that non maximum suppression produced
	double numberOfLabels = 0.0;

	int verticesWithoutLabel = 0;

	//prepare data
	//find the number of labels and the number of vertices, which were not labeled
	for(auto pVertex : mVertices){

		//if the feature vector has less than 24 elements, resize it to length 24
		if(24 > pVertex->getFeatureVectorLen()){

			pVertex->resizeFeatureVector(24);

		}

		//clear data, that might sit in the feature vectors, remaining from previous runs of the clustering algorithm
		//pVertex->setFeatureElement(20, 0.0);

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

	//number of clusters is now known

	//loop over every labeled group
	for(int labelIterator=1;labelIterator<=(int)numberOfLabels;labelIterator++){

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

			//smelly, comparing doubles for equality, however I don't know a better way at the moment
			//if((double)labelIterator == foundLabel){
			if(labelIterator == (int)foundLabel){

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
						adjacentVertex->getFeatureElement(19, &adjacentVertexClusterGroup);

						if(labelIterator == (int)adjacentVertexLabel){

							//so the neighbour belongs to the same label group

							double adjacentVertexClusterGroup;
							adjacentVertex->getFeatureElement(20, &adjacentVertexClusterGroup);

							if((int)currentVertexClusterGroup!=(int)adjacentVertexClusterGroup){

								//the current Vertex was found to be a border vertex

								borderVertexFound = true;

								//find out on what border the vertex lies

								//border case 1 and 2 (or 2 and 1)
								if( (((int)currentVertexClusterGroup == 1) && ((int)adjacentVertexClusterGroup == 2))
									|| (((int)currentVertexClusterGroup == 2 ) && ((int)adjacentVertexClusterGroup == 1)) ){

									currentVertex->setFeatureElement(21, 1.0);
									currentVertex->setFeatureElement(22, 2.0);

								//border case 2 and 3 (or 3 and 2)
								} else if ( (((int)currentVertexClusterGroup == 2) && ((int)adjacentVertexClusterGroup == 3))
									|| (((int)currentVertexClusterGroup == 3 ) && ((int)adjacentVertexClusterGroup == 2)) ){

									currentVertex->setFeatureElement(22, 2.0);
									currentVertex->setFeatureElement(23, 3.0);

								//border case 1 and 3 (or 3 and 1)
								} else if ( (((int)currentVertexClusterGroup == 3) && ((int)adjacentVertexClusterGroup == 1))
									|| (((int)currentVertexClusterGroup == 1 ) && ((int)adjacentVertexClusterGroup == 3)) ){

									currentVertex->setFeatureElement(21, 1.0);
									currentVertex->setFeatureElement(23, 3.0);
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
		//is initialized with a infinity placeholder
		double smallestSumOfDistances = numeric_limits<double>::max();

		//unassigned pointer may be dangerous
		//Werde ueberdenken
		//could be filled with dummy data
		Vertex* finalTetraederTop;
		Vertex* finalTetraederVertex1;
		Vertex* finalTetraederVertex2;
		Vertex* finalTetraederVertex3;

		//RANSAC begins
		for(int iteration=0;iteration<numberOfIterations;iteration++){

			double currentDistanceForThisCoice = 0.0;

			//randomly choose four vertices for RANSAC
			vector<Vertex*> fourRandomlyChosenBorderVertices;
			wERandomlyChooseVerticesFromVector(verticesBorderGroup, fourRandomlyChosenBorderVertices, 4);

			Vertex* randomlyChosenTetraederTop = fourRandomlyChosenBorderVertices[0];
			Vertex* randomlyChosenTetraederVertex1 = fourRandomlyChosenBorderVertices[1];
			Vertex* randomlyChosenTetraederVertex2 = fourRandomlyChosenBorderVertices[2];
			Vertex* randomlyChosenTetraederVertex3 = fourRandomlyChosenBorderVertices[3];

			for(auto borderVertex : verticesBorderGroup){

				//find minimum distance to one of the rays
				double minimumDistance = min(wEComputeShortestDistanceBetweenPointAndLine(borderVertex, randomlyChosenTetraederTop, randomlyChosenTetraederVertex1,
											wEComputeShortestDistanceBetweenPointAndLine(borderVertex, randomlyChosenTetraederTop, randomlyChosenTetraederVertex2),
											wEComputeShortestDistanceBetweenPointAndLine(borderVertex, randomlyChosenTetraederTop, randomlyChosenTetraederVertex3));
				currentDistanceForThisCoice += minimumDistance;
			}

			if(currentDistanceForThisCoice < smallestSumOfDistances){

				smallestSumOfDistances = currentDistanceForThisCoice;
				finalTetraederTop = randomlyChosenTetraederTop;
				finalTetraederVertex1 = randomlyChosenTetraederVertex1;
				finalTetraederVertex2 = randomlyChosenTetraederVertex2;
				finalTetraederVertex3 = randomlyChosenTetraederVertex3;

			}

		}
		//RANSAC now has noted down the current best fit for rays.

		//possible bordergroups
		/*
		12
		23
		31
		*/
		int borderGroupFinalTetVertex1;
		wEGetBorderGroupFromVertexByFeatureVector(finalTetraederVertex1, borderGroupFinalTetVertex1);

		int borderGroupFinalTetVertex2;
		wEGetBorderGroupFromVertexByFeatureVector(finalTetraederVertex2, borderGroupFinalTetVertex2);

		int borderGroupFinalTetVertex3;
		wEGetBorderGroupFromVertexByFeatureVector(finalTetraederVertex3, borderGroupFinalTetVertex3);

		int biggestDistanceGroup1;
		int biggestDistanceGroup2;
		int biggestDistanceGroup3;

		for(auto borderVertex : verticesBorderGroup){

			int currentborderVertexGroup;
			wEGetBorderGroupFromVertexByFeatureVector(borderVertex, currentborderVertexGroup);

			if(currentborderVertexGroup == 12){

			} else if (currentborderVertexGroup == 23){
			} else if (currentborderVertexGroup == 31){
			}
		}

		//project all borderVertices of that group onto the line formed by finalTetraederTop and finalTetraederVertex1
		//choose





		//take the the point furthest away of the finalTetraederTop



	}//all labeled groups have been worked upon

	return true;

}


bool experimentalFeatureVectorReordering(vector<Vertex*> &mVertices){

	cout << "unfinished" << endl;

	return true;

}
