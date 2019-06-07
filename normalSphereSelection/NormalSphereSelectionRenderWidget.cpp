#include "NormalSphereSelectionRenderWidget.h"
#include <cassert>
#include <iostream>
#include <QtMath>
#include <QMouseEvent>
#include <array>
#include "IcoSphereTree.h"

bool initShaderProgram(QOpenGLShaderProgram& program, const QString& vertexSource, const QString& fragmentSource)
{
	if(!program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexSource))
	{
		std::cerr << program.log().toStdString() << std::endl;
		return false;
	}

	if(!program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentSource))
	{
		std::cerr << program.log().toStdString() << std::endl;
		return false;
	}
	if(!program.link())
	{
		std::cerr << program.log().toStdString() << std::endl;
		return false;
	}
	return true;
}

QVector2D vec3ToSphereCoord(const QVector3D& vec)
{
	return QVector2D(
			(atan2f(vec.y(), vec.x()) / M_PI + 1.0f) * 0.5f,
			acosf(vec.z()) / M_PI
			);
}


QVector2D getScreenPosNormalized(int posX, int posY, int w, int h)
{
	float x = (static_cast<float>(posX) / static_cast<float>(w)) * 2.0f - 1.0f;
	float y = (static_cast<float>(posY) / static_cast<float>(h)) * 2.0f - 1.0f;

	return QVector2D(x , -y);
}

float raySphereIntersect(const QVector3D& r0, const QVector3D& rd)
{
	float b = 2.0f * QVector3D::dotProduct(rd, r0);
	float c = QVector3D::dotProduct(r0, r0) - 1.0f;

	float test = b*b - 4.0f*c;

	if (test < 0.0f) {
		return -1.0f;
	}
	return (-b - sqrtf(test))/ 2.0f;
}


NormalSphereSelectionRenderWidget::NormalSphereSelectionRenderWidget(QWidget* parent)
	: QOpenGLWidget(parent),  mFuncValTexture(QOpenGLTexture::Target2D), mSelectionTexture(QOpenGLTexture::Target2D),
	  mIcosphereIndices(QOpenGLBuffer::IndexBuffer),
	  mIcoSphereTree(6), mSelectionBuffer(0,0)
{
}

void NormalSphereSelectionRenderWidget::setRenderNormals(std::vector<float>& normals)
{
	mNormalUpload = std::move(normals);

	for(size_t i = 0; i<mNormalUpload.size(); i += 3)
	{
		if(std::isnan(mNormalUpload[i]) || std::isnan(mNormalUpload[i + 1]) || std::isnan(mNormalUpload[i+2]) )
			continue;

		size_t index = mIcoSphereTree.getNearestVertexIndexAt(QVector3D(mNormalUpload[i], mNormalUpload[i+1], mNormalUpload[i+2]));
		mIcoSphereTree.incData(index);
	}
}

void NormalSphereSelectionRenderWidget::setSelected(float nx, float ny, float nz)
{
	if(std::isnan(nx) || std::isnan(ny) || std::isnan(nz))
		return;

	size_t index = mIcoSphereTree.getNearestVertexIndexAt(QVector3D(nx, ny, nz));

	mIcoSphereTree.selectVertex(index);

	if(index < mSelectionBuffer.size())
		mSelectionBuffer[index] = 255;

	mUpdateSelectionTexture = true;
	update();
}

void NormalSphereSelectionRenderWidget::clearSelected()
{
	mIcoSphereTree.clearSelection();

	for(auto& val : mSelectionBuffer)
		val = 0;

	mUpdateSelectionTexture = true;
	update();
}

bool NormalSphereSelectionRenderWidget::isNormalSelected(float nx, float ny, float nz)
{
	if(std::isnan(nx) || std::isnan(ny) || std::isnan(nz))
		return false;

	size_t index = mIcoSphereTree.getNearestVertexIndexAt(QVector3D(nx, ny, nz));

	return mIcoSphereTree.isSelected(index);
}

void NormalSphereSelectionRenderWidget::setColorMapIndex(unsigned int index)
{
	mColorMapIndex = index;
	update();
}

void NormalSphereSelectionRenderWidget::refreshNormals()
{

	mIcosphereDataBuffer.bind();
	std::vector<float> dataBuffer(mIcoSphereTree.getVertexDataP()->size());

	for(size_t i = 0; i<dataBuffer.size(); ++i)
	{
		dataBuffer[i] = static_cast<float>((*mIcoSphereTree.getVertexDataP())[i]);
	}

	mMinData = dataBuffer[0];
	for(auto dat : dataBuffer)
	{
		mMinData = std::min(mMinData, dat);
	}

	mIcosphereDataBuffer.write(0,dataBuffer.data(), dataBuffer.size() * sizeof (float));
	mIcosphereDataBuffer.release();
	assert(glGetError() == GL_NO_ERROR);

	mNormalUpload.resize(0);
}

bool getSpherePoint(const QVector2D& screenCoordNorm, const QQuaternion& camRotation, QVector3D& retVec)
{
	if(screenCoordNorm.length() > 1.0 || screenCoordNorm.x() > 1.0 || screenCoordNorm.x() < -1.0 || screenCoordNorm.y() > 1.0 || screenCoordNorm.y() < -1.0)
			return false;

	QVector3D origin = camRotation * QVector3D(screenCoordNorm.x(), screenCoordNorm.y(), 1.5);
	QVector3D viewDirection = camRotation * QVector3D(0.0,0.0, -1.0);

	float interSectDist = raySphereIntersect(origin, viewDirection);

	if(interSectDist < 0.0)
		return false;

	retVec = (origin + viewDirection * interSectDist);
	return true;
}

void NormalSphereSelectionRenderWidget::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::MouseButton::LeftButton)
	{
		mArcBall.beginDrag( getScreenPosNormalized(event->x(), event->y() , mScreenWidth, mScreenHeight ));
		update();
	}
	else if(event->button() == Qt::MouseButton::RightButton)
	{
		auto posNormalized = getScreenPosNormalized(event->x(), event->y(), mScreenWidth, mScreenHeight);
		QVector3D sphereVec;
		if(getSpherePoint(posNormalized, mArcBall.getTransformationQuat().conjugated(), sphereVec ))
		{
			setSelected(sphereVec.x(), sphereVec.y(), sphereVec.z());
		}
	}
}

void NormalSphereSelectionRenderWidget::mouseReleaseEvent(QMouseEvent* event)
{
}

void NormalSphereSelectionRenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	if(event->buttons() & Qt::LeftButton)
	{
		mArcBall.drag( getScreenPosNormalized(event->x(), event->y() , mScreenWidth, mScreenHeight ) );
		update();
	}
	else if(event->buttons() & Qt::RightButton)
	{
		auto posNormalized = getScreenPosNormalized(event->x(), event->y(), mScreenWidth, mScreenHeight);
		QVector3D sphereVec;
		if(getSpherePoint(posNormalized, mArcBall.getTransformationQuat().conjugated(), sphereVec ))
		{
			setSelected(sphereVec.x(), sphereVec.y(), sphereVec.z());
		}
	}
}

void NormalSphereSelectionRenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	glClearColor(1.0,1.0,1.0,0.0);

	mProjectionMatrix.ortho(-1.0,1.0,-1.0,1.0,0.0,2);	//ortho matrix with unit qube for the sphere

	assert(glGetError() == GL_NO_ERROR);

	if(!initShaderProgram(mIcoSphereShader, tr(":GMShaders/normalSphere/IcoshphereShader.vert"), tr(":GMShaders/normalSphere/IcosphereShader.frag")))
		assert(false);

	//icosphere buffer
	mIcoSphereShader.bind();
	mIcoSphereVAO.create();
	mIcoSphereVAO.bind();

	assert(glGetError() == GL_NO_ERROR);

	mIcosphereBuffer.create();
	mIcosphereBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIcosphereBuffer.bind();

	assert(glGetError() == GL_NO_ERROR);

	auto vertexData = mIcoSphereTree.getVertexData();

	mIcosphereBuffer.allocate(vertexData.data(), vertexData.size() * sizeof (float));

	assert(glGetError() == GL_NO_ERROR);

	auto vertexLoc = mIcoSphereShader.attributeLocation("vPosition");
	mIcoSphereShader.enableAttributeArray(vertexLoc);
	mIcoSphereShader.setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 3, 0);

	assert(glGetError() == GL_NO_ERROR);

	mIcosphereBuffer.release();

	assert(glGetError() == GL_NO_ERROR);
	mIcosphereDataBuffer.create();
	mIcosphereDataBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIcosphereDataBuffer.bind();

	mIcosphereDataBuffer.allocate(vertexData.size() / 3 * sizeof (float));

	assert(glGetError() == GL_NO_ERROR);
	vertexLoc = mIcoSphereShader.attributeLocation("vData");
	mIcoSphereShader.enableAttributeArray(vertexLoc);
	mIcoSphereShader.setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 1, 0);

	assert(glGetError() == GL_NO_ERROR);
	mIcosphereIndices.create();
	mIcosphereIndices.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIcosphereIndices.bind();

	auto faceIndices = mIcoSphereTree.getFaceIndices();
	mIcosphereIndices.allocate(faceIndices.data(), faceIndices.size() * sizeof(unsigned int));

	mIcoSphereVAO.release();
	mIcosphereIndices.release();
	mIcosphereDataBuffer.release();
	mIcoSphereShader.release();

	QImage texImage(tr(":/GMShaders/funcvalmapsquare.png"));
	mFuncValTexture.setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	mFuncValTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	mFuncValTexture.setData(texImage.mirrored(), QOpenGLTexture::DontGenerateMipMaps);

	unsigned int texWidth = 64;
	unsigned int texHeight = 64;

	int maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

	while(texWidth < maxTexSize && texWidth * texHeight < vertexData.size() / 3)
	{
		texWidth <= texHeight ? texWidth *= 2 : texHeight *= 2;
	}

	mSelectionBuffer.resize(texWidth * texHeight, 0);

	mSelectionTexture.setFormat(QOpenGLTexture::R8_UNorm);
	mSelectionTexture.setSize(texWidth, texHeight);
	mSelectionTexture.setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
	mSelectionTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
	mSelectionTexture.allocateStorage();

	mSelectionTexture.setData(0,QOpenGLTexture::Red,QOpenGLTexture::UInt8, mSelectionBuffer.data());

}

void NormalSphereSelectionRenderWidget::resizeGL(int w, int h)
{
	assert(glGetError() == GL_NO_ERROR);
	glViewport(0,0,w,h);
	mScreenWidth = w;
	mScreenHeight = h;
	assert(glGetError() == GL_NO_ERROR);
}

void NormalSphereSelectionRenderWidget::paintGL()
{
	assert(glGetError() == GL_NO_ERROR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	assert(glGetError() == GL_NO_ERROR);
	if(!mNormalUpload.empty())
	{
		refreshNormals();
	}

	if(mUpdateSelectionTexture)
	{
		mUpdateSelectionTexture = false;
		mSelectionTexture.setData(0,QOpenGLTexture::Red,QOpenGLTexture::UInt8, mSelectionBuffer.data());
	}

	assert(glGetError() == GL_NO_ERROR);
	mIcoSphereShader.bind();
	mIcoSphereVAO.bind();

	//glCullFace(GL_BACK);
	QQuaternion quat = mArcBall.getTransformationQuat();

	QVector3D origin = quat.conjugated() * QVector3D(0.0,0.0, 1.5);
	QVector3D up = quat.conjugated() * QVector3D(0.0,1.0,0.0);

	mViewMatrix.setToIdentity();
	mViewMatrix.lookAt(origin, QVector3D(0.0,0.0,0.0), up);

	mIcoSphereShader.setUniformValue("uModelViewMatrix",mViewMatrix);
	mIcoSphereShader.setUniformValue("uProjectionMatrix", mProjectionMatrix);
	mIcoSphereShader.setUniformValue("uMaxData", static_cast<float>(mIcoSphereTree.getMaxData()));
	mIcoSphereShader.setUniformValue("uMinData", mMinData);
	mIcoSphereShader.setUniformValue("uColorMapIndex", static_cast<float>(mColorMapIndex));

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mFuncValTexture.textureId());

	mIcoSphereShader.setUniformValue("uFuncValTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mSelectionTexture.textureId());

	mIcoSphereShader.setUniformValue("uSelectionTexture", 1);
	mIcoSphereShader.setUniformValue("uTextureWidth", mSelectionTexture.width());

	glDrawElements(GL_TRIANGLES, mIcosphereIndices.size() / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

	assert(glGetError() == GL_NO_ERROR);

	mIcoSphereShader.release();
	mIcoSphereVAO.release();

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	assert(glGetError() == GL_NO_ERROR);
}
