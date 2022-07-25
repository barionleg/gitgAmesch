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

#include <GigaMesh/mesh/octree.h>

//forward declaration of functors, because they are used in class Octree
//we use functors, because we dont want overloading like this for each getter function:
//getnodelist(nodelist) and getnodelist(nodelist, cnode)
//and getnodelist(nodelist, cnode=mroot) is invalid c++03/11 if mroot is non-static member
class gnl;
class gnlinlvl;
template <class T>
class octreelineintersection;
template <class T>
class octreeleaflineintersection;
template <class T>
class octreetriangleintersection;




//! @param vertexlist  @param  center @param maxnr represents the maximum number of vertices per cube
//! @param edgelen contains the length of edge of the largest cube
//! Octree is primarily a octree of vertices. A Faceoctree can be created on the base of the vertex octree.
Octree::Octree(std::vector<Vertex*> &vertexlist,std::vector<Face*> &facelist, Vector3D* center, unsigned int maxnr, double edgelen, double EdgeLenMax, bool copyelements): mmaxnr(maxnr) {

    //max EdgeLen of current mesh
    mEdgeLenMax = EdgeLenMax;
    //current level is 0, as we have done no subdivision so far
    mmaxlevel = 0;
    //create root node
    mRootVertices = new Octnode(center, 0.5*edgelen);
    //assign elementlist to root node
    mRootVertices->mVertices = vertexlist;
    //mcopyelements true means that an element may copied to more than one node,
    //e.g. if it is in on the border of the nodes
    mcopyelements = copyelements;
    //start recursive initialization of octree
    initialize(mRootVertices, 1);

    //create face octree
    mRootFaces = new Octnode(center, 0.5*edgelen);
    //for all faces inside this vector we have to check in which nodes they intersect
    generateFacesOctreeHypothesis(mRootFaces,mRootVertices,1,&mIncompleteFaces);
    //delete dulicates
    sort(mIncompleteFaces.begin(),mIncompleteFaces.end());
    mIncompleteFaces.erase( unique( mIncompleteFaces.begin(), mIncompleteFaces.end()),mIncompleteFaces.end());
    correctFacesOctree(mIncompleteFaces);
}

//! destruct
Octree::~Octree() {
    std::vector<Octnode*> nodeVertPtrList;
    std::vector<Octnode*> nodeFacePtrList;
    getnodelist(nodeVertPtrList,VERTEX_OCTREE);
    getnodelist(nodeFacePtrList,FACE_OCTREE);
    // The following causes a CRASH, because .CLEAR() will try to delete the already deleted entries!!!
    //for(vector<Octnode*>::iterator it= nodelist.begin(); it!= nodelist.end(); ++it) {
    //	delete (*it);
    //}

    //delete Vertex octree
    for(Octnode* nodePtr : nodeVertPtrList)
    {
        delete nodePtr;
    }
    nodeVertPtrList.clear();

    //delete Face octree
    for(Octnode* nodePtr : nodeFacePtrList)
    {
        delete nodePtr;
    }
    nodeFacePtrList.clear();
}



void Octree::getnodelist(std::vector<Octnode*>& nodelist, eTreeType treeType) {
    if( treeType == VERTEX_OCTREE ){
        nodelist = mRootVertices->getNodeList();
    }
    else if( treeType == FACE_OCTREE){
        nodelist = mRootFaces->getNodeList();
    }
}

bool Octree::getNodesOfFace(std::vector<Octnode *> &nodelist, Face *face){
    std::vector<Octnode *> allLeafNodes;
    allLeafNodes = mRootFaces->getLeafNodes();

    for(Octnode* node : allLeafNodes){
        if(node->isFaceInside(face)){
            nodelist.push_back(node);
        }
    }
    if( nodelist.size() == 0){
        return false;
    }
    return true;
}
void Octree::getnodesinlevel(std::vector<Octnode*>& nodelist, unsigned int i) {
    //gnlinlvl<T> tmp(nodelist, i);
    //traverse_topdown(tmp);
}
//!get all nodes of the octree with vertices inside (leafnodes)
void Octree::getleafnodes(std::vector<Octnode*>& nodelist, eTreeType treeType) {
    if( treeType == VERTEX_OCTREE ){
        nodelist = mRootVertices->getLeafNodes();
    }
    else if( treeType == FACE_OCTREE){
        nodelist = mRootFaces->getLeafNodes();
    }

}
/**
//!get all nodes of the octree with vertices inside (leafnodes)
//get leaf nodes contained in cnode
void Octree::getleafnodes(std::vector<Octnode*>& nodelist, Octnode * cnode) {
    //gnlleaf<T> tmp(nodelist);
    //traverse_topdown(tmp, cnode);
}
**/
void Octree::getlineintersection(std::vector<Octnode*>& nodelist, Vector3D &a, Vector3D &b) {
    //octreelineintersection<T> tmp(nodelist, a, b);
    //traverse_topdown(tmp);
}
void Octree::getleaflineintersection(std::vector<Octnode*>& nodelist, Vector3D &a, Vector3D &b) {
    //octreeleaflineintersection<T> tmp(nodelist, a, b);
    //traverse_topdown(tmp);
}
void Octree::gettriangleintersection(std::vector<Octnode *>& nodelist, std::vector<Octnode*>& cnodelist,
                                     std::vector<Line> &drawlines, TriangularPrism& tri) {
    //octreetriangleintersection<T> tmp(nodelist, cnodelist, drawlines, tri);
    //traverse_topdown(tmp);
}




/*	std::vector<p> splitelements(std::vector<int> &v, size_t elementsPerThread) {
        std::vector<p> ranges;

        size_t range_count = (v.size()+1) / elementsPerThread+1;
        size_t ePT = v.size() / range_count;

        size_t i;

        it b = v.begin();

        for (i=0; i<v.size()-ePT; i+=ePT)
            ranges.push_back(std::make_pair(b+i, b+i+ePT));

        ranges.push_back(std::make_pair(b+i, v.end()));
        return ranges;
    }
*/
template <typename X>
std::vector<std::pair<typename std::vector<X>::iterator, typename std::vector<X>::iterator>>
splitthreads(std::vector<X> &v, size_t numberOfThreads) {

    std::vector<std::pair<typename std::vector<X>::iterator, typename std::vector<X>::iterator>> ranges;
    size_t h;
    typename std::vector<X>::iterator b = v.begin();
    size_t i;

    h = v.size()/numberOfThreads;

    for(i=0; i<numberOfThreads-1; ++i) {
        ranges.push_back(std::make_pair( b+(h*i), b+h*(i+1) ));
    }

    ranges.push_back(std::make_pair(b+i*h, v.end()));

    return ranges;
}


//helper function for detectselfintersectionst
// check the cases where one or two vertices are in common
// and check the (ugly) case where the ramaining vertex lies inside the other face
//return false means that we have found intersection between the two faces ,
//or that intersection is not possible at all, so we dont have to investigate further
//return true means we have found no  intersection and the other tests for intersection will be executed
bool Octree::twovert(std::vector<Face*>::iterator& kt, std::vector<Face*>::iterator& jt, std::vector<Face*>& sif,
                     Line& l1, Line& l2, Line& l3) {

    Vector3D tmp;
    int h;

    //if two faces have one vertex in common, dont check the edges
    //connected to that vertex, because its error prone due to numerical rounding
    if( (*kt)->getVertA() == (*jt)->getVertA() ||
            (*kt)->getVertB() == (*jt)->getVertA() ||
            (*kt)->getVertC() == (*jt)->getVertA() ) {

        //if two vertices in common, we just need to check if one of
        //the remaining vertices is in the other face
        if( (*kt)->getVertA() == (*jt)->getVertB() ||
                (*kt)->getVertB() == (*jt)->getVertB() ||
                (*kt)->getVertC() == (*jt)->getVertB() ) {
            tmp = (*jt)->getVertC()->getPositionVector();
            if ( (*kt)->pointintriangle(&tmp) ) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<std::endl;
#endif
                sif.push_back(*jt);
                sif.push_back(*kt);
            }
            return false;
        }
        if( (*kt)->getVertA() == (*jt)->getVertC() ||
                (*kt)->getVertB() == (*jt)->getVertC() ||
                (*kt)->getVertC() == (*jt)->getVertC() ) {
            tmp = (*jt)->getVertB()->getPositionVector();
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<std::endl;
#endif
                sif.push_back(*jt);
                sif.push_back(*kt);
            }
            return false;
        }



        h = l2.getLinePlaneIntersection(**kt, tmp);
        //h == 0 means, that the intersection of the line with the plane of the other triangle is
        // is between the two pints from the face describing the line
        if(h==0) {
            //check if point of intersection is in one ow the two triangles
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<" on Vert "<<tmp<<std::endl;
#endif
                sif.push_back(*jt);
            }
        }
        return false;
    }


    //if two faces have one vertex in common, dont check the edges
    //of the connected to that vertex, cause its error prone due to numerical rounding
    if( (*kt)->getVertA() == (*jt)->getVertB() ||
            (*kt)->getVertB() == (*jt)->getVertB() ||
            (*kt)->getVertC() == (*jt)->getVertB() ) {


        //if two vertices in common, nothing needs to be checked
        if( (*kt)->getVertA() == (*jt)->getVertA() ||
                (*kt)->getVertB() == (*jt)->getVertA() ||
                (*kt)->getVertC() == (*jt)->getVertA() ) {
            tmp = (*jt)->getVertC()->getPositionVector();
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<std::endl;
#endif
                sif.push_back(*jt);
                sif.push_back(*kt);
            }
            return false;
        }
        if(	(*kt)->getVertA() == (*jt)->getVertC() ||
                (*kt)->getVertB() == (*jt)->getVertC() ||
                (*kt)->getVertC() == (*jt)->getVertC() ) {
            tmp = (*jt)->getVertA()->getPositionVector();
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<std::endl;
#endif
                sif.push_back(*jt);
                sif.push_back(*kt);
            }
            return false;
        }




        h = l3.getLinePlaneIntersection(**kt, tmp);
        //h == 0 means, that the intersection of the line with the plane of the other triangle is
        // is between the two pints from the face describing the line
        if(h==0) {
            //check if point of intersection is in one ow the two triangles
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<" on Vert "<<tmp<<std::endl;
#endif
                sif.push_back(*jt);
            }
        }
        return false;
    }

    //if two faces have one vertex in common, dont check the edges
    //of the connected to that vertex, cause its error prone due to numerical rounding
    if( (*kt)->getVertA() == (*jt)->getVertC() ||
            (*kt)->getVertB() == (*jt)->getVertC() ||
            (*kt)->getVertC() == (*jt)->getVertC() ) {


        //if two vertices in common, nothing needs to be checked
        if( (*kt)->getVertA() == (*jt)->getVertB() ||
                (*kt)->getVertB() == (*jt)->getVertB() ||
                (*kt)->getVertC() == (*jt)->getVertB() ) {
            tmp = (*jt)->getVertA()->getPositionVector();
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<std::endl;
#endif
                sif.push_back(*jt);
                sif.push_back(*kt);
            }
            return false;
        }
        if ((*kt)->getVertA() == (*jt)->getVertA() ||
                (*kt)->getVertB() == (*jt)->getVertA() ||
                (*kt)->getVertC() == (*jt)->getVertA() ) {
            tmp = (*jt)->getVertB()->getPositionVector();
            if ( (*kt)->pointintriangle(&tmp)) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<std::endl;
#endif
                sif.push_back(*jt);
                sif.push_back(*kt);
            }
            return false;
        }


        h = l1.getLinePlaneIntersection(**kt, tmp);
        //h == 0 means, that the intersection of the line with the plane of the other triangle is
        // is between the two points from the face describing the line
        if(h==0) {
            if ( (*kt)->pointintriangle(&tmp) ) {
#ifdef DEBUG
                std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<" with "<< (*kt)->getIndex()
                        <<" on Vert "<<tmp<<std::endl;
#endif
                sif.push_back(*jt);
            }
        }
        return false;
    }

    return true;
}

/**
    //! @param[out] sif contains pointers to self intersectiing faces
    //! @param[in] p the pair of iterator begin to end for this thread
    void detectselfintersectionst(std::vector<Face*>& sif,
        std::pair< std::vector<Octnode*>::iterator,
                             std::vector<Octnode*>::iterator > & p) {

        Vector3D tmp;
        int h;

        //typedef std::chrono::high_resolution_clock clock;
        //typedef std::chrono::milliseconds milliseconds;



        //check all edges of faces (may intersect plane of other face)
        for(auto it=p.first; it != p.second; ++it) {

            //clock::time_point t2 = clock::now();
            for(auto jt = (*it)->mElements.begin(); jt != (*it)->mElements.end(); ++jt ) {

                //clock::time_point t0 = clock::now();
                //Line l1( *((*jt)->getVertA()), *((*jt)->getVertB()) );
                //Line l2( *((*jt)->getVertB()), *((*jt)->getVertC()) );
                //Line l3( *((*jt)->getVertC()), *((*jt)->getVertA()) );

                //clock::time_point t1 = clock::now();
                //milliseconds total_ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
                //std::cout<<"Creation of Lines lasted: "<<total_ms.count() << "ms\n";

                for(auto kt = (*it)->mElements.begin(); kt != (*it)->mElements.end(); ++kt ) {

                    //do not check the edges of the face itself
                    if(*kt != *jt) {


                        //if ( twovert(kt, jt, sif, l1, l2, l3) ) {


                            //if none of the special cases where nothing or only partial check is necessary
                            //occurs, than check all edges for intersection with the other face

                            /h = l1.getLinePlaneIntersection(**kt, tmp);
                            //h == 0 means, that the intersection of the line with the plane of the other triangle is
                            // is between the two points from the face describing the line
                            if(h==0) {
                                if ( (*kt)->pointontriangle(&tmp) ) {
                                    #ifdef DEBUG
                                    std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<" with "<< (*kt)->getIndex()
                                       <<" on Vert "<<tmp<<std::endl;
                                    #endif
                                    sif.push_back(*jt);
                                    sif.push_back(*kt);
                                    continue;
                                }
                            }
                            h = l2.getLinePlaneIntersection(**kt, tmp);
                            //h == 0 means, that the intersection of the line with the plane of the other triangle is
                            // is between the two pints from the face describing the line
                            if(h==0) {
                                //check if point of intersection is on one of the two triangles
                                if ( (*kt)->pointontriangle(&tmp) ) {
                                    #ifdef DEBUG
                                    std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<" on Vert "<<tmp<<std::endl;
                                    #endif
                                    sif.push_back(*jt);
                                    sif.push_back(*kt);
                                    continue;
                                }
                            }
                            h = l3.getLinePlaneIntersection(**kt, tmp);
                            //h == 0 means, that the intersection of the line with the plane of the other triangle is
                            // is between the two pints from the face describing the line
                            if(h==0) {
                                //check if point of intersection is on one of the two triangles
                                if ( (*kt)->pointontriangle(&tmp) ) {
                                    #ifdef DEBUG
                                    std::cout<<"TEST self intersected face: "<<(*jt)->getIndex() <<"with "<< (*kt)->getIndex() <<" on Vert "<<tmp<<std::endl;
                                    #endif
                                    sif.push_back(*jt);
                                    sif.push_back(*kt);
                                    continue;
                                }
                            }
                        }

                    }
                }
            }
            //clock::time_point t3 = clock::now();
            //milliseconds total_ms = std::chrono::duration_cast<milliseconds>(t3 - t2);
            //std::cout<<"One Octnode lasted: "<<total_ms.count() << "ms\n";
        }
    }
**/


//! @param[out] sif contains pointers to self intersectiing faces
//!
void Octree::detectselfintersections(std::vector<Face*>& sif) {
    std::vector<Octnode*> nodelist;
    getleafnodes(nodelist,VERTEX_OCTREE);

    using p = std::pair<std::vector<Octnode*>::iterator,
    std::vector<Octnode*>::iterator>;

    //#ifdef THREADS
#define NUM_THREADSA           7


    std::vector<p> res= splitthreads(nodelist, NUM_THREADSA);
    std::array<std::thread, NUM_THREADSA> threads;

    for(size_t i=0; i<NUM_THREADSA; ++i) {
        //threads[i] = std::thread( &Octree::detectselfintersectionst, this, std::ref(sif), std::ref(res[i]) );
    }
    for(auto& thread : threads){
        thread.join();
    }

}






//! prints number of levels and number of nodes in each level and total number of nodes
bool Octree::dumpInfo() {
    std::cout<<"-------------------------------------------------------"<<std::endl;
    std::cout<<"--------------- Octree Information --------------------"<<std::endl<<std::endl;
    std::cout<<"Octree maximum number of Vertices per cube: "<<mmaxnr<<std::endl;
    std::cout<<"Octree maximum depth:       "<<mmaxlevel<<std::endl;
    std::vector<Octnode*> nodelist;
    size_t temp = 0;
    unsigned int i = 0;
    size_t totalnodes=0;
    do {
        temp = nodelist.size();
        getnodesinlevel(nodelist, i);
        ++i;
        totalnodes += nodelist.size() - temp;
        std::cout<<"In Level " << i-1 << " are " << nodelist.size() - temp <<" Octnodes "<<std::endl;
    }
    while (temp != nodelist.size() && i<= mmaxlevel);
    std::cout<<"In total there are "<< totalnodes << " Octnodes in the whole Octree"<<std::endl;
    std::cout<<"--------------- End Octree Information ----------------"<<std::endl;
    std::cout<<"-------------------------------------------------------"<<std::endl;
    nodelist.clear();
    return true;
}

/**
    //! prints all centers of octnodes
    bool dumpInfov() {
        std::vector<Octnode*> nodelist;
        getnodelist(nodelist);
        for ( typename std::vector<Octnode*>::iterator it = nodelist.begin(); it!=nodelist.end(); ++it) {
            (*it)->mcenter.dumpInfo();
            std::cout<<(*it)->mscale<<std::endl;
        }
        nodelist.clear();
        return true;
    }

    //! prints all centers of octnodes and corresponding vertices
    bool dumpInfovlong() {
        std::vector<Octnode<Vertex*>*> nodelist;
        getnodelist(nodelist);
        for ( typename std::vector<Octnode*>::iterator it = nodelist.begin(); it!=nodelist.end(); ++it) {
            (*it)->mcenter.dumpInfo();
            std::cout<<(*it)->mscale<<std::endl;
            for ( typename std::vector<T>::iterator jt = (*it)->mVertices.begin(); jt!=(*it)->mVertices.end(); ++jt) {
                (*jt)->dumpInfo();
            }
        }
        nodelist.clear();
        return true;
    }
**/



//!oointers - object octree intersection
// returns true if there is an intersection between object and cnode
// has to be implemented for each object type, as long as there is no general approach
// to decide if a object intersects cube or not
//for Vertex*
void Octree::oointers(Vertex* object, Octnode* cnode) {
    double x = object->getX();
    double y = object->getY();
    double z = object->getZ();

    for (int j=0; j<8; j++) {
        if ( fabs( x - cnode->mchildren[j]->mCube.mcenter.getX() ) <= 0.5*cnode->mCube.mscale &&
             fabs( y - cnode->mchildren[j]->mCube.mcenter.getY() ) <= 0.5*cnode->mCube.mscale &&
             fabs( z - cnode->mchildren[j]->mCube.mcenter.getZ() ) <= 0.5*cnode->mCube.mscale   ) {

            cnode->mchildren[j]->mVertices.push_back(object);
            if(mcopyelements==false) {
                break;
            }
        }
    }
}

//for Face*
void Octree::oointers(Face* object, Octnode* cnode) {
    Vector3D c = object->getCenterOfGravity();
    double x = c.getX();
    double y = c.getY();
    double z = c.getZ();

    for (int j=0; j<8; j++) {
        if ( fabs( x - cnode->mchildren[j]->mCube.mcenter.getX() ) <= cnode->mCube.mscale &&
             fabs( y - cnode->mchildren[j]->mCube.mcenter.getY() ) <= cnode->mCube.mscale &&
             fabs( z - cnode->mchildren[j]->mCube.mcenter.getZ() ) <= cnode->mCube.mscale   ) {

            //cnode->mchildren[j]->mElements.push_back(object);
            if(mcopyelements==false) {
                break;
            }
        }
    }
}

//helper function for recursive construction (we do not want to call the constructor recursively)
bool Octree::initialize(Octnode* cnode, unsigned int clevel) {

    //set current level (depth of octree)
    mmaxlevel = clevel;

    //decide if to further subdivide or mark octnode as leaf
    if ( mmaxnr < cnode->mVertices.size() &&
         clevel < mmaxlevelpermitted){ //&&
         //mEdgeLenMax < cnode->mCube.mscale ) {

        //create children
        for ( unsigned int i=0; i<8; ++i ) {
            cnode->mchildren[i] = new Octnode(cnode, i);
        }

        //assign vertices to children
        for (typename std::vector<Vertex*>::iterator it = cnode->mVertices.begin(); it!=cnode->mVertices.end(); ++it) {
            oointers(*it, cnode);
        }


        //delete vertices in node above
        cnode->mVertices.clear();


        //next level
        for ( unsigned int i=0; i<8; ++i ) {
            initialize(cnode->mchildren[i], clevel + 1);
        }

    }
    //mark as leaf
    else {
        cnode->misleaf = true;
    }
    return true;
}

void Octree::generateFacesOctreeHypothesis(Octnode* parentNodeFace,Octnode* parentNodeVertex, unsigned int treeLevel, std::vector<Face*> *incompleteFaces){
    //traverse through the vertex octree
    //create the same nodes for the face octree
    //if the node is a leaf node, then add all involved faces of the vertices to the new node
    if( parentNodeVertex->misleaf ){
        parentNodeFace->misleaf = true;
        // get all faces of the vertices inside the node
        for( unsigned int i=0; i < parentNodeVertex->mVertices.size(); i++){
            std::vector<Face*> facesOfVertex;
            parentNodeVertex->mVertices[i]->getFaces(&facesOfVertex);
            //not using insert because you have to check if the face is already in the node
            for(Face* face : facesOfVertex){
                //only add face if not already in mfaces
                if(!(parentNodeFace->mFaces.find(face) != parentNodeFace->mFaces.end())){
                    parentNodeFace->mFaces.insert(face);
                    //check here if the face is completely in octnode/cube --> if not it is an interesting face for correction
                    if(!parentNodeFace->mCube.isFaceCompletelyInCube(face)){
                        incompleteFaces->push_back(face);
                    }
                }
            }

            //parentNodeFace->mFaces.insert(parentNodeFace->mFaces.end(), facesOfVertex.begin(), facesOfVertex.end());
        }
    }
    else{
        //create children
        parentNodeFace->misleaf = false;
        for ( unsigned int i=0; i<8; ++i ) {
            parentNodeFace->mchildren[i] = new Octnode(parentNodeFace, i);
        }

        //next level
        for ( unsigned int i=0; i<8; ++i ) {
            std::vector<Face*> tmpIncompleteFaces;
            generateFacesOctreeHypothesis(parentNodeFace->mchildren[i],parentNodeVertex->mchildren[i], treeLevel + 1, &tmpIncompleteFaces);
            incompleteFaces->insert(incompleteFaces->begin(),tmpIncompleteFaces.begin(),tmpIncompleteFaces.end());
        }
    }

}

void Octree::correctFace(Face *face)
{
    //get the nodes where the face is aligned
    std::vector<Octnode*> faceNodes;
    if (!getNodesOfFace(faceNodes,face)) {
        //the face isn't in any node --> faces octree is not correct
        //TODO: error handling
    }

    //face is not completely inside the node
    //get the level of the octree in that the face is completely inside
    Octnode* parent;
    parent = faceNodes[0]->mparent;
    while(!parent->mCube.isFaceCompletelyInCube(face)){
        parent = parent->mparent;
        //loop emergency exit
        if( parent->mlevel == 1 ){
            break;
        }
    }
    //add face to all nodes that intersects their cube
    addFaceToAllIntersectedChildren(parent,face);

}


void Octree::correctFacesOctree(std::vector<Face *> &facelist){
    // check all problematic faces (facelist = all faces with more then one node )

    unsigned int i = 0;
    while(i < facelist.size()){
        std::vector<std::thread> threads(NUM_THREADSA);
        // spawn n threads:
        for (int threadId = 0; threadId < NUM_THREADSA; threadId++) {
            if(i < facelist.size()){
                i++;
            }
            threads[threadId] = std::thread(&Octree::correctFace, this, std::ref(facelist[i]));

        }
        //wait until the threads are finished
        for (auto& th : threads) {
            th.join();
        }
    }
}

void Octree::addFaceToAllIntersectedChildren(Octnode *parentNode, Face *face){
    //get all children that the face intersects
    for ( unsigned int i=0; i<8; ++i ) {
        if( parentNode->mchildren[i]->mCube.trianglecubeintersection(face) ){
            // deeper into the tree if not a leaf node
            if(parentNode->mchildren[i]->misleaf){
                //add if face is not inside
                if(!parentNode->mchildren[i]->isFaceInside(face)){
                  parentNode->mchildren[i]->mFaces.insert(face);
                }
            }
            else{
                addFaceToAllIntersectedChildren(parentNode->mchildren[i],face);
            }

        }
    }
}



/// maximum depth of octree
unsigned int mmaxlevel;
// mmaxnr represents the maximum number of vertices per cube
unsigned int mmaxnr;

//at this level no further subdivision is permitted
static constexpr unsigned int mmaxlevelpermitted = 15 ;

//! @param copyelements ==true => elements on the border of a cube are copied to all adjacent cubes
//! if copyelements ==false => elements on the border of a cube are copied to a single cube
//! should be true for correct collision detection etc.
bool mcopyelements;

double mEdgeLenMax;







//functors that may passed to octree::traverse_topdown for doing some actions, e.g. information retrieval


//! functor that returns a list of all nodes when passed to octree::traverse_topdown
//!
class gnl{

public:
    gnl(std::vector<Octnode*>& nodelist_):nodelist(nodelist_) {}

    bool operator() (Octnode * cnode) {
        if (cnode->misleaf == false) {
            nodelist.push_back( cnode );
            return true;
        }
        else {
            nodelist.push_back( cnode );
            return false;
        }
    }


    std::vector<Octnode *>& nodelist;

};



//! functor that returns a list of all nodes in level mlevel when passed to octree::traverse_topdown

class gnlinlvl{

public:
    gnlinlvl(std::vector<Octnode*>& nodelist_, unsigned int level):nodelist(nodelist_), mlevel(level) {}

    bool operator() (Octnode * cnode) {

        if ( (cnode->misleaf == false) && (mlevel > cnode->mlevel) ) {
            return true;
        }
        if (mlevel == cnode->mlevel) {
            nodelist.push_back( cnode );
            return false;
        }

        if ( cnode->misleaf == true) {
            return false;
        }
        //should not be reached
        return false;
    }

    std::vector<Octnode *>& nodelist;
    unsigned int mlevel;

};

//! functor that returns a list of all leaf-nodes when passed to octree::traverse_topdown

class gnlleaf{

public:
    gnlleaf(std::vector<Octnode*>& nodelist_):nodelist(nodelist_) {}

    bool operator() (Octnode * cnode) {
        if (cnode->misleaf == false) {
            //nodelist.push_back( cnode );
            return true;
        }
        else {
            nodelist.push_back( cnode );
            return false;
        }
    }


    std::vector<Octnode *>& nodelist;

};





//! returns nodelist of nodes which are intersected by a line through a and b
template <class T>
class octreelineintersection {
public:
    octreelineintersection(std::vector<Octnode*>& nodelist_, Vector3D &a_, Vector3D &b_):nodelist(nodelist_), a(a_), b(b_) {}

    bool operator() (Octnode * cnode) {
        if(cnode->mCube.linecubeintersection(&a,&b)) {
            if (cnode->misleaf == true) {
                nodelist.push_back( cnode );
                return false;
            }
            else {
                nodelist.push_back( cnode );
                return true;
            }
        }
        return false;
    }


    std::vector<Octnode *>& nodelist;
    Vector3D &a, &b;

};

//! returns nodelist of nodes which are intersected by a line through a and b
//! only leaf ndoes are returned
template <class T>
class octreeleaflineintersection {
public:
    octreeleaflineintersection(std::vector<Octnode*>& nodelist_, Vector3D &a_, Vector3D &b_):nodelist(nodelist_), a(a_), b(b_) {}

    bool operator() (Octnode * cnode) {
        if(cnode->mCube.linecubeintersection(&a,&b)) {
            if (cnode->misleaf == true) {
                nodelist.push_back( cnode );
                return false;
            }
            else {
                //nodelist.push_back( cnode );
                return true;
            }
        }
        return false;
    }


    std::vector<Octnode *>& nodelist;
    Vector3D &a, &b;

};


/**
//! returns list of leaf nodes which are intersected by triangular prism defined as Vector3D[6] tri
//! @param nodelist_ vector of leaf nodes, which have intersection with triangle
//! @param cnodelist_ vector of leaf nodes, which are containing the triangle
template <class T>
class octreetriangleintersection {
public:
    octreetriangleintersection(std::vector<Octnode *>& nodelist_, std::vector<Octnode*>& cnodelist_,
                             std::vector<Line> &drawlines_, TriangularPrism& tri_): \
                             nodelist(nodelist_), cnodelist(cnodelist_), drawlines(drawlines_), tri(tri_) {}

    bool operator() (Octnode * current) {

            relativePosition tmp;
            tmp = current->mCube.CubeisintriangularPrism(tri, drawlines);

            switch(tmp) {
                case CONTAINED:
                    if(current->misleaf) {
                        cnodelist.push_back(current);
                        return false;
                    }
                    else {
                        return true;
                    }
                case INTERSECTION:
                    if(current->misleaf) {
                        nodelist.push_back(current);
                        return false;
                    }
                    else {
                        return true;
                    }
                case DISJOINT:
                    return false;
                //should not be reached
                default:
                    std::cout<<"OCTREE triangle intersection something is incorrect"<<std::endl;
                    return false;

                //should not be reached
                std::cout<<"OCTREE triangle intersection something is incorrect"<<std::endl;
                return false;
            }
            //should not be reached
            std::cout<<"OCTREE triangle intersection something is incorrect"<<std::endl;
            return false;
        }
};
**/
