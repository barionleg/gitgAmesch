#ifndef ICOSPHERETREE_H
#define ICOSPHERETREE_H

#include <QVector3D>
#include <vector>
#include <memory>
#include <array>
#include <unordered_set>

struct IcoSphereTreeFaceNode
{
		void setVertIndices(size_t v0, size_t v1, size_t v2);
		std::array<size_t, 3> vertexIndices{0,0,0};
		std::vector< std::unique_ptr<IcoSphereTreeFaceNode> > childNodes;
};

class IcoSphereTree
{
	public:
		IcoSphereTree(unsigned int subdivision = 0);
		std::vector<unsigned int> getFaceIndices(int subdivisionLevel = -1) const;
		[[nodiscard]] std::vector<float> getVertexData() const;

		size_t getNearestVertexIndexAt(const QVector3D& position) const;

		//returns true if ray intersects icosphere. If true, the index of the nearest vertex is stored in 'index'
		//last parameter toggles, if vertices "behind" the ray should also be considered
		bool getNearestVertexFromRay(const QVector3D& rayOrigin, const QVector3D& rayDirection, size_t& index, bool rayIsLine = true) const;

		void selectVertex(size_t index);
		void deselectVertex(size_t index);
		void clearSelection();
		bool isSelected(size_t index);

		void incData(size_t index);
		unsigned int getMaxData() const;

		std::vector<unsigned int>* getVertexDataP() { return &mVertexData;}

	private:
		void subdivide(unsigned int subdivisions = 1);

		std::array<IcoSphereTreeFaceNode, 20> mRootFaces;
		std::vector<QVector3D> mVertices;
		std::vector<unsigned int> mVertexData;	//function-value associated to this bucket => num normals in this bucket
		std::unordered_set<size_t> mSelectedVertices;

		unsigned int mMaxData = 0;
};

#endif // ICOSPHERETREE_H
