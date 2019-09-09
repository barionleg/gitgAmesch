#include "TexturedMeshRenderer.h"
#include "../mesh/mesh.h"
#include <array>
#include <list>
#include <string>

#include "glmacros.h"

TexturedMeshRenderer::TexturedMeshRenderer()
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

bool TexturedMeshRenderer::init(const std::vector<std::string>& textureNames)
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

	mTextures.resize(textureNames.size());

	for(size_t i = 0; i<textureNames.size(); ++i)
	{
		QImage texImage(textureNames[i].c_str());
		mTextures[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
		mTextures[i]->setData(texImage.mirrored(), QOpenGLTexture::GenerateMipMaps);
	}

	mIsInitialized = true;
	return true;
}

//assumes that vao and shader of the TexturedMesh Renderer is bound
//should only be called from render
void TexturedMeshRenderer::setUpVertexBuffer(QOpenGLBuffer& vertexBuffer)
{
	vertexBuffer.bind();

	mShader->setAttributeBuffer( "vPosition", GL_FLOAT, offsetof(TexturedMeshVertex, pos), 3, sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vPosition" );

	mShader->setAttributeBuffer( "vNormal", GL_FLOAT, offsetof(TexturedMeshVertex, normal), 3, sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vNormal" );

	mShader->setAttributeBuffer( "vUV", GL_FLOAT, offsetof(TexturedMeshVertex, uv), 2 , sizeof(TexturedMeshVertex) );
	mShader->enableAttributeArray( "vUV" );
}

void TexturedMeshRenderer::render(const QMatrix4x4& projectionMatrix, const QMatrix4x4& modelViewMatrix, TexturedMesh& texturedMesh, const LightInfo& lightInfo)
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
	mShader->setUniformValue("uLightDirectionFixedCamera", lightInfo.lightDirFixedCam  );
	mShader->setUniformValue("uLightDirectionFixedWorld" , lightInfo.lightDirFixedWorld);
	mShader->setUniformValue("uLightEnabled"             , lightInfo.lightEnabled      );
	mShader->setUniformValue("FixedCam_DiffuseProduct"   , lightInfo.fixedCamDiffuse   );
	mShader->setUniformValue("FixedCam_SpecularProduct"  , lightInfo.fixedCamSpecular  );
	mShader->setUniformValue("FixedWorld_DiffuseProduct" , lightInfo.fixedWorldDiffuse );
	mShader->setUniformValue("FixedWorld_SpecularProduct", lightInfo.fixedWorldSpecular);
	mShader->setUniformValue("AmbientProduct"            , lightInfo.ambient           );
	mShader->setUniformValue("Shininess"                 , static_cast<GLfloat>(lightInfo.shininess));

	mGL.glActiveTexture(GL_TEXTURE0);
	PRINT_OPENGL_ERROR("active texture");

	unsigned int elementSize = TexturedMesh::getVertexElementSize();

	for(auto& bufferPair : texturedMesh.getVertexBuffers())
	{
		if(bufferPair.first > mTextures.size())
			continue;

		//bind textureid of bufferPair 1
		mGL.glBindTexture(GL_TEXTURE_2D, mTextures[bufferPair.first]->textureId());

		for(auto& buffer : bufferPair.second)
		{
			setUpVertexBuffer(buffer);
			mGL.glDrawArrays(GL_TRIANGLES, 0, buffer.size() / elementSize);
		}

		mTextures[bufferPair.first]->release();
	}

	//mGL.glDrawArrays(GL_TRIANGLES, 0, numVertices);

	PRINT_OPENGL_ERROR("draw Arrays");
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

	for(auto& texture : mTextures)
	{
		texture->destroy();
		delete texture;
	}

	mTextures.clear();

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
