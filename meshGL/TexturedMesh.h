#ifndef TEXTUREDMESH_H
#define TEXTUREDMESH_H

#include <vector>
#include <map>
#include <list>
#include <QOpenGLBuffer>

class Mesh;
class Face;
class Vertex;

class TexturedMesh
{
	public:
		TexturedMesh() = default;

		//The destructor needs to be called with a valid OpenGLContext. If this is not possible (e.g. allocated on stack), call destroy with a valid context
		~TexturedMesh();

		TexturedMesh(const TexturedMesh&  other) = delete;
		TexturedMesh(      TexturedMesh&& other) = delete;

		TexturedMesh& operator=(const TexturedMesh&  other) = delete;
		TexturedMesh& operator=(      TexturedMesh&& other) = delete;

		//Needs to be called with a valid OpenGLContext
		void establishStructure(Mesh* mesh);

		//Needs to be called with a valid OpenGLContext
		void destroy();

	private:
		void generateBuffers(const std::map<unsigned char, std::list<Face*> >& faces);
		std::map<unsigned char, std::list<QOpenGLBuffer>> mVertexBuffers;
};

#endif // TEXTUREDMESH_H
