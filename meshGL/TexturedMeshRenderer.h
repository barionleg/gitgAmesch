#ifndef TEXTUREDMESHRENDERER_H
#define TEXTUREDMESHRENDERER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <string>

class TexturedMeshRenderer
{
	public:
		TexturedMeshRenderer();
		~TexturedMeshRenderer();

		bool init(QOpenGLBuffer& vertexBuffer, const std::string& textureName);
		void render(const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelViewMatrix, QOpenGLBuffer& vertexBuffer, unsigned int numVertices);
		void destroy();

		struct TexturedMeshVertex {
			GLfloat pos[3];
			GLfloat normal[3];
			GLfloat uv[2];
		};

		bool isInitialized() const;

	private:

		bool initShader();
		bool mIsInitialized = false;

		QOpenGLVertexArrayObject mVAO;
		QOpenGLShaderProgram* mShader = nullptr;
		QOpenGLTexture mTexture;
		QOpenGLFunctions_3_3_Core mGL;
};

#endif // TEXTUREDMESHRENDERER_H
