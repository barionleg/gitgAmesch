#include <iostream>
#include <cmath>
#include <stdlib.h>

#include "geodesicneighbours.h"

using namespace std;

GeodesicNeighbours::GeodesicNeighbours() {
	//! Constructor.
	//cout << "[GeodesicNeighbours] constructed." << endl;
}

GeodesicNeighbours::~GeodesicNeighbours() {
	//! Destructor.
	for( uint i=0; i<edgeListSorted.size(); i++ ) {
		free( edgeListSorted[i] );
	}
	edgeListSorted.clear();
	//cout << "[GeodesicNeighbours] destructed." << endl;
}

bool GeodesicNeighbours::addEdge( Edge* frontEdge, Face* fromFace ) {
	geodesicFrontEdge *edgeToInsert = (geodesicFrontEdge*) calloc( sizeof( geodesicFrontEdge ), 1 );

	edgeToInsert->frontEdge = frontEdge;
	edgeToInsert->fromFace  = fromFace;

	// check if we have the vertices A and B and their geodesic distances in our 
    // list and if they need an update:
	map<Vertex*,float>::iterator pairA;
	pairA = vertexGeodesicDist.find( frontEdge->getVertA() );
	map<Vertex*,float>::iterator pairB;
	pairB = vertexGeodesicDist.find( frontEdge->getVertB() );
	//cout << "[GeodesicNeighbours] add(new)0: " << pairA->first->getIdx() << "-" << pairB->first->getIdx() << " ... " << pairA->second << " - " << pairB->second << endl;

	float distLow, distHigh;
	if( pairA->second < pairB->second ) {
		distLow  = pairA->second;
		distHigh = pairB->second;
	} else {
		distLow  = pairB->second;
		distHigh = pairA->second;
	}
	
	bool inserted = false;
	// if the list is empty we just add the edge:
	if( edgeListSorted.size() == 0 ) {
		edgeListSorted.insert( edgeListSorted.begin(), edgeToInsert );
		inserted = true;
	} else {
	// we insert sorted by the shorter geodesic distance (distLow)
		vector<geodesicFrontEdge*>::iterator it;
		for( it=edgeListSorted.begin(); (it<edgeListSorted.end() && !inserted); it++ ) {
			Vertex* vertAList = (*it)->frontEdge->getVertA();
			Vertex* vertBList = (*it)->frontEdge->getVertB();
			float distAList = vertexGeodesicDist.find( vertAList )->second;
			float distBList = vertexGeodesicDist.find( vertBList )->second;
			float distLowList, distHighList;
			if( distAList < distBList ) {
				distLowList  = distAList;
				distHighList = distBList;
			} else {
				distLowList  = distBList;
				distHighList = distAList;
			}
			if( distLowList <= distLow ) {
				if( distLowList == distLow ) {
					if( distHighList > distHigh ) {
						// this typically happens for the lowest geodesic distance
						// within the marching front 
						// if equal, we have to consider distHigh for sorting:
						edgeListSorted.insert( it, edgeToInsert );
						inserted = true;
					} else {
						edgeListSorted.insert( ++it, edgeToInsert );
						inserted = true;
					}
				}
			} else {
				edgeListSorted.insert( it, edgeToInsert );
				inserted = true;
			}
		}
		if( !inserted ) {
			edgeListSorted.insert( edgeListSorted.end(), edgeToInsert );
			inserted = true;
		}
	}

	return inserted;
}

void GeodesicNeighbours::addBorderEdge( Edge* newBorderEdge ) {
	borderEdges.push_back( newBorderEdge );
}

void GeodesicNeighbours::addVertex( pair<Vertex*,float> pairVertex ) {
	map<Vertex*,float>::iterator it;
	it = vertexGeodesicDist.find( pairVertex.first );
	if( it == vertexGeodesicDist.end() ) {
		// insert:
		vertexGeodesicDist.insert( pairVertex );
	} else {
		// rewrite if new value is lower:
		if( it->second > pairVertex.second ) {
			//cout << "[GeodesicNeighbours] Update Geodesic for " << pairVertex.first->getIdx() << ": " << it->second << " <= " <<  pairVertex.second << endl;
			//dumpHashInfo();
			it->second = pairVertex.second;
		}
	}
}

void GeodesicNeighbours::addFace( Face* faceVisited ) {
	facesIncluded.insert( faceVisited );
}

bool GeodesicNeighbours::has( Edge* edgeSome ) {
	vector<geodesicFrontEdge*>::iterator it;
	for( it=edgeListSorted.begin(); it<edgeListSorted.end(); it++ ) {
		if( (*it)->frontEdge == edgeSome ) {
			cout << "[GeodesicNeighbours] has Edge." << endl;
			return true;
		}
	}
	cout << "[GeodesicNeighbours] has not Edge." << endl;
	return false;
}

bool GeodesicNeighbours::has( Face* faceVisited ) {
	set<Face*>::iterator it;
	it = facesIncluded.find( faceVisited );
	if( it == facesIncluded.end() ) {
		return false;
	}
	//cout << "[GeodesicNeighbours] walking in circles prevented." << endl;
	return true;
}

bool GeodesicNeighbours::remove( Edge* edgeSome ) {
	vector<geodesicFrontEdge*>::iterator it;
	for( it=edgeListSorted.begin(); it<edgeListSorted.end(); it++ ) {
		if( (*it)->frontEdge == edgeSome ) {
			edgeListSorted.erase( it );
			//cout << "[GeodesicNeighbours] had Edge." << endl;
			return true;
		}
	}
	//cout << "[GeodesicNeighbours] has not Edge." << endl;
	return false;
}

void GeodesicNeighbours::get( Edge** frontEdge, float* distA, float* distB, Face** fromFace ) {
	*frontEdge = edgeListSorted[0]->frontEdge;
	*fromFace  = edgeListSorted[0]->fromFace;

	map<Vertex*,float>::iterator it;
	it = vertexGeodesicDist.find( edgeListSorted[0]->frontEdge->getVertA() );
	*distA     = it->second;
	it = vertexGeodesicDist.find( edgeListSorted[0]->frontEdge->getVertB() );
	*distB     = it->second;

	edgeListSorted.erase( edgeListSorted.begin() );
}

map<Vertex*,float> GeodesicNeighbours::getGeodesicDistances() {
	return vertexGeodesicDist;
}

float GeodesicNeighbours::getMaxGeodesicDistance() {
	float maxDist = 0.0;
	map<Vertex*,float>::iterator it;

	for( it=vertexGeodesicDist.begin(); it != vertexGeodesicDist.end(); it++ ) {
		if( it->second > maxDist ) {
			maxDist = it->second;
		}
	}
	return maxDist;
}

list<Vertex*> GeodesicNeighbours::getBorderVertices() {
	list<Vertex*> verticesBorder;

	vector<Edge*>::iterator it;
	for( it=borderEdges.begin(); it<borderEdges.end(); it++ ) {
		verticesBorder.push_back( (*it)->getVertA() );
		verticesBorder.push_back( (*it)->getVertB() );
	}
	// remove duplicates:
	verticesBorder.sort();
	verticesBorder.unique();

	return verticesBorder;
}

uint GeodesicNeighbours::sortedStackSize() {
	return edgeListSorted.size();
}

// DEBUGING --------------------------------------------------------------------

void GeodesicNeighbours::dumpHashInfo() {
	if( edgeListSorted.size() <= 0 ) {
		cout << "[GeodesicNeighbours] edgeListSorted is empty." << endl;
		return;
	} 

	vector<geodesicFrontEdge*>::iterator it;
	cout << "[GeodesicNeighbours] edgeListSorted contains: " << endl;
	for( it=edgeListSorted.begin(); it<edgeListSorted.end(); it++ ) {
		cout << "                     ";
		cout << (*it)->frontEdge->getVertA()->getIndexOriginal() << " - " << (*it)->frontEdge->getVertB()->getIndexOriginal();
		Vertex* vertAList = (*it)->frontEdge->getVertA();
		Vertex* vertBList = (*it)->frontEdge->getVertB();
		float distAList = vertexGeodesicDist.find( vertAList )->second;
		float distBList = vertexGeodesicDist.find( vertBList )->second;
		cout << "... " << distAList << " / " << distBList;
/*
		if( (*it)->frontEdge->isBorder() ) {
			cout << " on BORDER. ";
		}
		if( (*it)->frontEdge->isNonManifold() ) {
			cout << " non manifold. ";
		}
*/
		cout << endl;
	}
/*
	set<Face*>::iterator itFace;
	cout << "[GeodesicNeighbours] facesIncluded contain: ";
	for( itFace=facesIncluded.begin(); itFace!=facesIncluded.end(); itFace++ ) {
		cout << (*itFace)->getIdx() << ", ";
	}
	cout << endl;
*/
}

