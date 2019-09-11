#include "TexturedMesh.h"

#include "../mesh/mesh.h"

#include <limits>
#include "glmacros.h"

struct TexturedVertex {
		float pos[3];
		float norm[3];
		float uv[2];
};

TexturedMesh::~TexturedMesh()
{
	destroy();
}

void TexturedMesh::establishStructure(Mesh* mesh)
{
	if(isCreated())
		return;

	std::map< unsigned char, std::list<Face*> > facePerMat;

	{
		std::vector<Face*> faces;
		mesh->getFaceList(&faces);

		//generate face list per texture
		for(auto facePtr : faces)
		{
			facePerMat[facePtr->getTextureId()].push_back(facePtr);
		}
	}

	generateBuffers(facePerMat);
	PRINT_OPENGL_ERROR("Error creating buffers");
}

void TexturedMesh::destroy()
{
	if(!mVertexBuffers.empty())
	{
		for(auto& bufferList : mVertexBuffers)
		{
			for(auto & buffer : bufferList.second)
			{
				buffer.destroy();
			}
		}
	}

	mVertexBuffers.clear();
}

bool TexturedMesh::isCreated() const
{
	return !mVertexBuffers.empty();
}

std::map<unsigned char, std::list<QOpenGLBuffer> >& TexturedMesh::getVertexBuffers()
{
	return mVertexBuffers;
}

unsigned int TexturedMesh::getVertexElementSize()
{
	return sizeof(TexturedVertex);
}

void TexturedMesh::generateBuffers(const std::map<unsigned char, std::list<Face*> >& faces)
{
	constexpr auto maxFacesPerBuffer = std::numeric_limits<int>::max() / (3 * sizeof(TexturedVertex));
	for(const auto& faceList : faces)
	{
		int texId = faceList.first;

		//the size of a single buffer may exceed maxInt. In this case we need to split it up into multiple smaller ones
		unsigned char buffersPerTexture = std::ceil(static_cast<double>(faceList.second.size()) / static_cast<double>(maxFacesPerBuffer));
		int facesPerBuffer = faceList.second.size() / buffersPerTexture;
		int remainder = faceList.second.size() % buffersPerTexture;

		std::vector<TexturedVertex> vertices((facesPerBuffer + 1) * 3);
		int vertLoc = 0;
		int facesAdded = 0;

		for(const auto& face : faceList.second)
		{
			std::array<Vertex*,3> faceVertices{face->getVertA(), face->getVertB(), face->getVertC()};
			auto uvs = face->getUVs();

			//copy vertices to local structure
			for(size_t j = 0; j<3; ++j)
			{
				vertices[vertLoc].uv[0] = uvs[j*2];
				vertices[vertLoc].uv[1] = uvs[j*2 + 1];
				vertices[vertLoc].pos[0] = faceVertices[j]->getX();
				vertices[vertLoc].pos[1] = faceVertices[j]->getY();
				vertices[vertLoc].pos[2] = faceVertices[j]->getZ();

				vertices[vertLoc].norm[0] = faceVertices[j]->getNormalX();
				vertices[vertLoc].norm[1] = faceVertices[j]->getNormalY();
				vertices[vertLoc].norm[2] = faceVertices[j]->getNormalZ();

				++vertLoc;
			}

			++facesAdded;

			if(facesAdded >= (facesPerBuffer + (remainder > 0)) )
			{
				mVertexBuffers[texId].push_back(QOpenGLBuffer());
				auto vertBuf = mVertexBuffers[texId].back();

				vertBuf.create();
				vertBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
				vertBuf.bind();
				vertBuf.allocate(vertices.data(), vertLoc * sizeof (TexturedVertex));

				vertLoc = 0;
				facesAdded = 0;
				if(remainder > 0)
					--remainder;
			}
		}

		mVertexBuffers[texId].push_back(QOpenGLBuffer());
		auto vertBuf = mVertexBuffers[texId].back();

		vertBuf.create();
		vertBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
		vertBuf.bind();
		vertBuf.allocate(vertices.data(), vertLoc * sizeof (TexturedVertex));
	}
}
