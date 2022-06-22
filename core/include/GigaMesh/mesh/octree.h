/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OCTREE_H
#define OCTREE_H

#include <utility>
#include <thread>
#include <array>
#include <chrono>
#include "vector3d.h"
#include "octnode.h"
#include "rectbox.h"
#include "face.h"


class Octree {

public:
    //! @param vertexlist  @param  center @param maxnr represents the maximum number of vertices per cube
    //! @param edgelen contains the length of edge of the largest cube
    //! Octree is primarily a octree of vertices. A Faceoctree can be created on the base of the vertex octree.
    Octree(std::vector<Vertex*> &vertexlist, Vector3D* center, unsigned int maxnr, double edgelen, double EdgeLenMax, bool copyelements=true);

	//! destruct
    ~Octree();

    template <class C>
    bool traverse_topdown(C& functor);

    template <class C>
    bool traverse_topdown(C& functor, Octnode* cnode);
    void getnodelist(std::vector<Octnode*>& nodelist);
    void getnodesinlevel(std::vector<Octnode*>& nodelist, unsigned int i);
    void getleafnodes(std::vector<Octnode*>& nodelist);
    //get leaf nodes contained in cnode
    //void getleafnodes(std::vector<Octnode*>& nodelist, Octnode * cnode);
    void getlineintersection(std::vector<Octnode*>& nodelist, Vector3D &a, Vector3D &b);
    void getleaflineintersection(std::vector<Octnode*>& nodelist, Vector3D &a, Vector3D &b);
    void gettriangleintersection(std::vector<Octnode *>& nodelist, std::vector<Octnode*>& cnodelist,
                                 std::vector<Line> &drawlines, TriangularPrism& tri);


	bool twovert(std::vector<Face*>::iterator& kt, std::vector<Face*>::iterator& jt, std::vector<Face*>& sif,
                 Line& l1, Line& l2, Line& l3);


	void detectselfintersectionst(std::vector<Face*>& sif,
        std::pair< std::vector<Octnode*>::iterator,
                             std::vector<Octnode*>::iterator > & p);


	//! @param[out] sif contains pointers to self intersectiing faces
    void detectselfintersections(std::vector<Face*>& sif);
	//! prints number of levels and number of nodes in each level and total number of nodes
    bool dumpInfo();


    /**
	//! prints all centers of octnodes
    bool dumpInfov();

	//! prints all centers of octnodes and corresponding vertices
    bool dumpInfovlong();
**/

private:


    void oointers(Vertex* object, Octnode* cnode);
    void oointers(Face* object, Octnode* cnode);

	//helper function for recursive construction (we do not want to call the constructor recursively)
    bool initialize(Octnode* cnode, unsigned int clevel);


	/// pointer to the root node
    Octnode* mroot;

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


};


#endif // OCTREE_H
