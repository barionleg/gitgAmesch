#include "TexturedMeshRenderer.h"
#include "../mesh/mesh.h"
#include <array>
#include <list>
#include <string>

#include "glmacros.h"

TexturedMeshRenderer::TexturedMeshRenderer() : mTexture(QOpenGLTexture::Target2D)
{

}

TexturedMeshRenderer::~TexturedMeshRenderer()
{
	if(mIsInitialized)
	{
		destroy();
		mIsInitialized = false;
	}
}

bool TexturedMeshRenderer::init(const std::string& textureName)
{
	if(mIsInitialized)
		return true;

	mGL.initializeOpenGLFunctions();

	GLint prevVAO;
	mGL.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);
	//init shader

	if(!initShader())
		return false;

	mVAO.create();
	mVAO.bind();

	mVAO.release();

	mGL.glBindVertexArray(prevVAO);


	QImage texImage(textureName.c_str());
	mTexture.setData(texImage.mirrored(), QOpenGLTexture::GenerateMipMaps);

	mIsInitialized = true;
	return true;
}

void TexturedMeshRenderer::setUpVertexBuffer(QOpenGLBuffer& vertexBuffer)
{
	GLint prevVAO;
	mGL.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);

	mShader->bind();
	mVAO.bind();

	vertexBuffer.bind();

	mShader->setAttributeBuffer( "vPosition", GL_FLOAT, offsetof(TexturedMeshVertex, pos), 3, sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vPosition" );

	mShader->setAttributeBuffer( "vNormal", GL_FLOAT, offsetof(TexturedMeshVertex, normal), 3, sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vNormal" );

	mShader->setAttributeBuffer( "vUV", GL_FLOAT, offsetof(TexturedMeshVertex, uv), 2 , sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vUV" );

	mVAO.release();
	vertexBuffer.release();
	mShader->release();

	mGL.glBindVertexArray(prevVAO);
}

void TexturedMeshRenderer::render(const QMatrix4x4& projectionMatrix, const QMatrix4x4& modelViewMatrix, unsigned int numVertices)
{
	if(!mIsInitialized)
		return;

	PRINT_OPENGL_ERROR("start of textured mesh renderer");

	GLint prevVAO;
	mGL.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVAO);

	mShader->bind();
	mVAO.bind();

	//set uniforms
	mShader->setUniformValue("modelViewMat", modelViewMatrix);
	mShader->setUniformValue("projectionMat", projectionMatrix);

	mShader->setUniformValue("uTexture", 0);
	mShader->setUniformValue("uLightWorld", QVector3D(0.0,0.0,0.0));

	mGL.glActiveTexture(GL_TEXTURE0);
	PRINT_OPENGL_ERROR("active texture");
	mGL.glBindTexture(GL_TEXTURE_2D, mTexture.textureId());
	PRINT_OPENGL_ERROR("bind texture");

	mGL.glDrawArrays(GL_TRIANGLES, 0, numVertices);

	PRINT_OPENGL_ERROR("draw Arrays");
	mTexture.release();
	mVAO.release();
	mShader->release();

	mGL.glBindVertexArray(prevVAO);
	mGL.glBindTexture(GL_TEXTURE_2D, 0);
	PRINT_OPENGL_ERROR("release texture");
}

void TexturedMeshRenderer::destroy()
{
	mShader->removeAllShaders();

	mVAO.destroy();

	mTexture.destroy();

	delete mShader;
}

bool TexturedMeshRenderer::initShader()
{

	mShader = new QOpenGLShaderProgram;

	if(!mShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/GMShaders/textureMesh/texturemesh.vert"))
	{
		QString errMsg = mShader->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[TexturedMeshRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "TEXMESH" << "/vert): " << errMsg.toStdString() << std::endl;
		mShader->removeAllShaders();
		return false;
	}

	if(!mShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GMShaders/textureMesh/texturemesh.frag"))
	{
		QString errMsg = mShader->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[TexturedMeshRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "TEXMESH" << "/frag): " << errMsg.toStdString() << std::endl;
		mShader->removeAllShaders();
		return false;
	}

	if(!mShader->link())
	{
		QString errMsg = mShader->log();
		errMsg = errMsg.left( errMsg.indexOf( "***"));
		std::cerr << "[TexturedMeshRenderer::" << __FUNCTION__ << "] ERROR: compiling shader program (" << "TEXMESH" << "/program): " << errMsg.toStdString() << std::endl;
		mShader->removeAllShaders();
		return false;
	}
	return true;
}

bool TexturedMeshRenderer::isInitialized() const
{
	return mIsInitialized;
}
