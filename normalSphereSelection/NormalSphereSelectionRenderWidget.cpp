#include "NormalSphereSelectionRenderWidget.h"
#include <cassert>
#include <iostream>
#include <QtMath>
#include <QMouseEvent>
#include <array>
#include <algorithm>
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
			(atan2f(vec.y(), vec.x()) / M_PI + 1.0F) * 0.5F,
			acosf(vec.z()) / M_PI
			);
}


QVector2D getScreenPosNormalized(int posX, int posY, int w, int h)
{
	float x = (static_cast<float>(posX) / static_cast<float>(w)) * 2.0F - 1.0F;
	float y = (static_cast<float>(posY) / static_cast<float>(h)) * 2.0F - 1.0F;


	//respect aspect ratio
	if(w < h)
		y *= static_cast<float>(h) / static_cast<float>(w);
	else
		x *= static_cast<float>(w) / static_cast<float>(h);

	return QVector2D(x , -y);
}

float raySphereIntersect(const QVector3D& r0, const QVector3D& rd)
{
	float b = 2.0F * QVector3D::dotProduct(rd, r0);
	float c = QVector3D::dotProduct(r0, r0) - 1.0F;

	float test = b*b - 4.0F*c;

	if (test < 0.0F) {
		return -1.0F;
	}
	return (-b - sqrtf(test))/ 2.0F;
}


NormalSphereSelectionRenderWidget::NormalSphereSelectionRenderWidget(QWidget* parent)
	: QOpenGLWidget(parent),  mFuncValTexture(QOpenGLTexture::Target2D), mSelectionTexture(QOpenGLTexture::Target2D),
	  mIcosphereIndices(QOpenGLBuffer::IndexBuffer),
	  mIcoSphereTree(6), mSelectionBuffer(0,0)
{
}

NormalSphereSelectionRenderWidget::~NormalSphereSelectionRenderWidget()
{
	makeCurrent();

	mFuncValTexture.destroy();
	mSelectionTexture.destroy();

	mIcoSphereVAO.destroy();

	mIcosphereBuffer.destroy();
	mIcosphereDataBuffer.destroy();
	mIcosphereIndices.destroy();

	mIcoSphereShader->removeAllShaders();
	delete mIcoSphereShader;

	doneCurrent();
}

void NormalSphereSelectionRenderWidget::setRenderNormals(std::vector<float>& normals)
{
	mNormalUpload = std::move(normals);

	for(size_t i = 0; i<mNormalUpload.size(); i += 3)
	{
		if(std::isnan(mNormalUpload[i]) || std::isnan(mNormalUpload[i + 1]) || std::isnan(mNormalUpload[i+2]) )
			continue;

		size_t index = mIcoSphereTree.getNearestVertexIndexAt(Vector3D(mNormalUpload[i], mNormalUpload[i+1], mNormalUpload[i+2]));
		mIcoSphereTree.incData(index);
	}
}

void NormalSphereSelectionRenderWidget::setSelected(double nx, double ny, double nz)
{
	if(std::isnan(nx) || std::isnan(ny) || std::isnan(nz))
		return;

	size_t index = mIcoSphereTree.getNearestVertexIndexAt(Vector3D(nx, ny, nz));

	if(mSelectionMask != 0)
		mIcoSphereTree.selectVertex(index);
	else
		mIcoSphereTree.deselectVertex(index);

	if(index < mSelectionBuffer.size())
		mSelectionBuffer[index] = mSelectionMask;

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

bool NormalSphereSelectionRenderWidget::isNormalSelected(double nx, double ny, double nz)
{
	if(std::isnan(nx) || std::isnan(ny) || std::isnan(nz))
		return false;

	size_t index = mIcoSphereTree.getNearestVertexIndexAt(Vector3D(nx, ny, nz));

	return mIcoSphereTree.isSelected(index);
}

void NormalSphereSelectionRenderWidget::setColorMapIndex(unsigned int index)
{
	mColorMapIndex = index;
	update();
}

void NormalSphereSelectionRenderWidget::refreshNormals()
{
	auto icoData = mIcoSphereTree.getVertexDataP();

	std::vector<float> dataBuffer;

	float minData = *(std::min_element(icoData->begin(), icoData->end()));
	float maxData = mIcoSphereTree.getMaxData();

	std::transform(icoData->begin(), icoData->end(), std::back_inserter(dataBuffer),
				   [minData, maxData](float data) -> float {return (data - minData) / (maxData - minData);}
	);

	mIcosphereDataBuffer.bind();
	mIcosphereDataBuffer.write(0,dataBuffer.data(), dataBuffer.size() * sizeof (float));
	mIcosphereDataBuffer.release();
	assert(glGetError() == GL_NO_ERROR);

	mNormalUpload.resize(0);
}



float NormalSphereSelectionRenderWidget::selectionRadius() const
{
	return mSelectionRadius;
}

void NormalSphereSelectionRenderWidget::setSelectionRadius(float selectionRadius)
{
	mSelectionRadius = std::min(selectionRadius, 10.0F);
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

void NormalSphereSelectionRenderWidget::selectAt(int xCoord, int yCoord)
{
	if(mSelectionRadius == 1.0F)
	{
		auto posNormalized = getScreenPosNormalized(xCoord, yCoord, mScreenWidth, mScreenHeight);
		QVector3D sphereVec;
		if(getSpherePoint(posNormalized, mArcBall.getTransformationQuat().conjugated(), sphereVec ))
		{
			setSelected(sphereVec.x(), sphereVec.y(), sphereVec.z());
		}
	}

	else
	{
		//TODO: do this in normalized coordinates? Otherwise it depends on the window-size...
		for(float x = -mSelectionRadius; x < mSelectionRadius; x += 1.0F)
		{
			for(float y = -mSelectionRadius; y < mSelectionRadius; y += 1.0F)
			{
				if(QVector2D(x,y).lengthSquared() > mSelectionRadius * mSelectionRadius)
					continue;

				auto posNormalized = getScreenPosNormalized(xCoord + x, yCoord + y, mScreenWidth, mScreenHeight);
				QVector3D sphereVec;
				if(getSpherePoint(posNormalized, mArcBall.getTransformationQuat().conjugated(), sphereVec ))
				{
					setSelected(sphereVec.x(), sphereVec.y(), sphereVec.z());
				}
			}
		}
	}
}

void NormalSphereSelectionRenderWidget::setUpperQuantil(float upperQuantil)
{
	mUpperQuantil = upperQuantil;
	update();
}

void NormalSphereSelectionRenderWidget::setInvertFuncVal(bool invertFuncVal)
{
	mInvertFuncVal = invertFuncVal;
	update();
}

GLubyte NormalSphereSelectionRenderWidget::selectionMask() const
{
	return mSelectionMask;
}

void NormalSphereSelectionRenderWidget::setSelectionMask(const GLubyte& selectionMask)
{
	mSelectionMask = selectionMask;
}

void NormalSphereSelectionRenderWidget::setRotation(const QQuaternion& rotQuat)
{
	mArcBall.setTransformationQuat(rotQuat);
	update();
}

QQuaternion NormalSphereSelectionRenderWidget::getRotation()
{
	return mArcBall.getTransformationQuat();
}

void NormalSphereSelectionRenderWidget::setScaleNormals(bool enable)
{
	mScaleNormals = enable;
	update();
}

void NormalSphereSelectionRenderWidget::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::MouseButton::LeftButton)
	{
		mArcBall.beginDrag( getScreenPosNormalized(event->x(), event->y() , mScreenWidth, mScreenHeight ));
		update();
	}
	else if(event->button() == Qt::MouseButton::RightButton && !mScaleNormals)
	{
		selectAt(event->x(), event->y());
		update();
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

		emit rotationChanged(mArcBall.getTransformationQuat());

		update();
	}
	else if(event->buttons() & Qt::RightButton && !mScaleNormals)
	{
		selectAt(event->x(), event->y());
		update();
	}
}

void NormalSphereSelectionRenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	mIcoSphereShader = new QOpenGLShaderProgram;

	glClearColor(1.0F,1.0F,1.0F,0.0F);

	mProjectionMatrix.ortho(-1.0F,1.0F,-1.0F,1.0F,-1.0F,3.0F);	//ortho matrix with unit qube for the sphere

	assert(glGetError() == GL_NO_ERROR);

	if(!initShaderProgram(*mIcoSphereShader, QString(":GMShaders/normalSphere/IcoshphereShader.vert"), QString(":GMShaders/normalSphere/IcosphereShader.frag")))
		assert(false);

	//icosphere buffer
	mIcoSphereShader->bind();
	mIcoSphereVAO.create();
	mIcoSphereVAO.bind();

	assert(glGetError() == GL_NO_ERROR);

	mIcosphereBuffer.create();
	mIcosphereBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIcosphereBuffer.bind();

	assert(glGetError() == GL_NO_ERROR);

	auto vertexData = mIcoSphereTree.getVertices();

	mIcosphereBuffer.allocate(vertexData.data(), vertexData.size() * sizeof (float));

	assert(glGetError() == GL_NO_ERROR);

	auto vertexLoc = mIcoSphereShader->attributeLocation("vPosition");
	mIcoSphereShader->enableAttributeArray(vertexLoc);
	mIcoSphereShader->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 3, 0);

	assert(glGetError() == GL_NO_ERROR);

	mIcosphereBuffer.release();

	assert(glGetError() == GL_NO_ERROR);
	mIcosphereDataBuffer.create();
	mIcosphereDataBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIcosphereDataBuffer.bind();

	mIcosphereDataBuffer.allocate(vertexData.size() / 3 * sizeof (float));

	assert(glGetError() == GL_NO_ERROR);
	vertexLoc = mIcoSphereShader->attributeLocation("vData");
	mIcoSphereShader->enableAttributeArray(vertexLoc);
	mIcoSphereShader->setAttributeBuffer(vertexLoc, GL_FLOAT, 0, 1, 0);

	assert(glGetError() == GL_NO_ERROR);
	mIcosphereIndices.create();
	mIcosphereIndices.setUsagePattern(QOpenGLBuffer::StaticDraw);
	mIcosphereIndices.bind();

	auto faceIndices = mIcoSphereTree.getFaceIndices();
	mIcosphereIndices.allocate(faceIndices.data(), faceIndices.size() * sizeof(unsigned int));

	mIcoSphereVAO.release();
	mIcosphereIndices.release();
	mIcosphereDataBuffer.release();
	mIcoSphereShader->release();

	QImage texImage(QString(":/GMShaders/funcvalmapsquare.png"));
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

	float width = 1.0F;
	float height = 1.0f;

	//maintain aspect ratio
	if(w < h)
		height = static_cast<float>(h) / static_cast<float>(w);
	else
		width = static_cast<float>(w) / static_cast<float>(h);

	mProjectionMatrix.setToIdentity();
	mProjectionMatrix.ortho(-width,width,-height,height,-1.0F,3.0F);	//ortho matrix with unit qube for the sphere

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
	mIcoSphereShader->bind();
	mIcoSphereVAO.bind();

	//glCullFace(GL_BACK);
	QQuaternion quat = mArcBall.getTransformationQuat();

	QVector3D origin = quat.conjugated() * QVector3D(0.0F,0.0F, 1.5F);
	QVector3D up = quat.conjugated() * QVector3D(0.0F,1.0F,0.0F);

	mViewMatrix.setToIdentity();
	mViewMatrix.lookAt(origin, QVector3D(0.0F,0.0F,0.0F), up);

	mIcoSphereShader->setUniformValue("uModelViewMatrix",mViewMatrix);
	mIcoSphereShader->setUniformValue("uProjectionMatrix", mProjectionMatrix);
	mIcoSphereShader->setUniformValue("uColorMapIndex", static_cast<float>(mColorMapIndex));
	mIcoSphereShader->setUniformValue("invertFuncVal", mInvertFuncVal);
	mIcoSphereShader->setUniformValue("uUpperQuantil", mUpperQuantil);

	float normalScale = mScaleNormals ? 0.7f : 1.0F;
	mIcoSphereShader->setUniformValue("uNormalScale", normalScale );

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFuncValTexture.textureId());

	mIcoSphereShader->setUniformValue("uFuncValTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mSelectionTexture.textureId());

	mIcoSphereShader->setUniformValue("uSelectionTexture", 1);
	mIcoSphereShader->setUniformValue("uTextureWidth", mSelectionTexture.width());

	glDrawElements(GL_TRIANGLES, mIcosphereIndices.size() / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

	assert(glGetError() == GL_NO_ERROR);

	mIcoSphereShader->release();
	mIcoSphereVAO.release();

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_CULL_FACE);
	assert(glGetError() == GL_NO_ERROR);
}
