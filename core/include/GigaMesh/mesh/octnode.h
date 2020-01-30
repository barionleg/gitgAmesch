#ifndef OCTNODE_H
#define OCTNODE_H

#include "vertex.h"
#include "vector3d.h"
#include "line.h"
#include "cube.h"

// IMPORTANT lesson about templates:
//==================================
// "The only portable way of using templates at the moment is to implement them in header files by using inline functions."
// -- C++ standard library: a tutorial and handbook.

// Sets default values
// ----------------------------------------------------
#define OCTNODEDEFAULTS   \
	misleaf( false ), \
	mchildren()


template <class T>

class Octnode {

public:
	// Constructor for the root node.
	Octnode( Vector3D* center, double scale ) : \
	        OCTNODEDEFAULTS,                    \
	        mparent( nullptr ),                    \
	        mlevel( 0 ),                        \
	        mCube( *center, scale ) {
		// do nothing
	};

	// Constructor for a child node.
	Octnode( Octnode* parent, int nr ) : \
	       OCTNODEDEFAULTS,              \
	       mparent( parent ),            \
	       mlevel( parent->mlevel+1 ),   \
	       mCube( Cube(mparent->mCube.mcenter, 0.5*mparent->mCube.mscale).getVertex(nr), 0.5*mparent->mCube.mscale ) {
		// do nothing
	};

	// Destructor
//	~Octnode();

	//is this node a leaf node?
	bool misleaf;
	/// pointers to child nodes
	Octnode* mchildren[8];
	/// pointer to parent node
	Octnode* mparent;
	/// the tree-depth of this node
	unsigned int mlevel;
	 /// the centerpoint of this node
	Cube mCube;

	/// vertices inside this node
	std::vector<T> mElements;

	/// comparison function to sort nodes by level
	static bool compareoctnode( Octnode<T>* rNode1, Octnode<T>* rNode2 ) {
		return ( rNode1->mlevel < rNode2->mlevel );
	}
};


#endif // OCTNODE_H
