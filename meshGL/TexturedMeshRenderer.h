#ifndef TEXTUREDMESHRENDERER_H
#define TEXTUREDMESHRENDERER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <string>
#include "TexturedMesh.h"

class TexturedMeshRenderer
{
	public:
		TexturedMeshRenderer();
		~TexturedMeshRenderer();

		TexturedMeshRenderer(const TexturedMeshRenderer& other) = delete;
		TexturedMeshRenderer(const TexturedMeshRenderer&& other) = delete;

		TexturedMeshRenderer& operator=(const TexturedMeshRenderer& other) = delete;
		TexturedMeshRenderer& operator=(const TexturedMeshRenderer&& other) = delete;

		struct LightInfo
		{
				QVector4D fixedCamDiffuse;
				QVector4D fixedCamSpecular;
				QVector4D fixedWorldDiffuse;
				QVector4D fixedWorldSpecular;
				QVector4D ambient;
				QVector3D lightDirFixedCam;
				QVector3D lightDirFixedWorld;
				double shininess = 1.0;
				bool lightEnabled = false;
		};

		bool init(const std::vector<std::string>& textureNames);
		void render(const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelViewMatrix, TexturedMesh& texturedMesh, const LightInfo& lightInfo);

		void destroy();

		struct TexturedMeshVertex {
			GLfloat pos[3];
			GLfloat normal[3];
			GLfloat uv[2];
		};

		bool isInitialized() const;

	private:

		void setUpVertexBuffer(QOpenGLBuffer& vertexBuffer);
		bool initShader();
		bool mIsInitialized = false;

		QOpenGLVertexArrayObject mVAO;
		QOpenGLShaderProgram* mShader = nullptr;
		std::vector<QOpenGLTexture*> mTextures;
		QOpenGLFunctions_3_3_Core mGL;
};

#endif // TEXTUREDMESHRENDERER_H
