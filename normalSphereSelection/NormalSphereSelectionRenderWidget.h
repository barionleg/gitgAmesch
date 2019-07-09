#ifndef NormalSphereSELECTIONRenderWIDGET_H
#define NormalSphereSELECTIONRenderWIDGET_H

#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <vector>

#include "ArcBall.h"
#include "IcoSphereTree.h"

class NormalSphereSelectionRenderWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
		Q_OBJECT
	public:
		explicit NormalSphereSelectionRenderWidget(QWidget* parent = nullptr);
		~NormalSphereSelectionRenderWidget() override;

		//moves the data from normals to internal data => normals is afterward empty!
		void setRenderNormals(std::vector<float>& normals);

		void setSelected(double nx, double ny, double nz);
		void clearSelected();
		bool isNormalSelected(double nx, double ny, double nz);
		// QWidget interface

		void setColorMapIndex(unsigned int index);
		float selectionRadius() const;
		void setSelectionRadius(float selectionRadius);

		GLubyte selectionMask() const;
		void setSelectionMask(const GLubyte& selectionMask);

		void setRotation(const QQuaternion& rotQuat);
		QQuaternion getRotation();

		void setScaleNormals(bool enable);
	signals:
		void rotationChanged(QQuaternion quat);

	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

		// QOpenGLWidget interface
	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;

	private:
		void initializeUnitSphereBuffer();
		void refreshNormals();
		void selectAt(int xCoord, int yCoord);

		QMatrix4x4 mViewMatrix;
		QMatrix4x4 mProjectionMatrix;

		QOpenGLTexture mFuncValTexture;
		QOpenGLTexture mSelectionTexture;

		QOpenGLBuffer mIcosphereBuffer;
		QOpenGLBuffer mIcosphereDataBuffer;
		QOpenGLBuffer mIcosphereIndices;
		QOpenGLVertexArrayObject mIcoSphereVAO;

		//create QOpenGLShaderprogram on heap, because there is no destroy function and we have to rely on a valid context on destruction
		QOpenGLShaderProgram* mIcoSphereShader = nullptr;

		bool mUpdateSelectionTexture = false;

		std::vector<float> mNormalUpload;
		std::vector<GLubyte> mSelectionBuffer;

		ArcBall mArcBall;

		int mScreenWidth = 0;
		int mScreenHeight = 0;

		IcoSphereTree mIcoSphereTree;
		float mMinData = 0.0f;

		unsigned int mColorMapIndex = 0;

		float mSelectionRadius = 1.0f;
		GLubyte mSelectionMask = 255;
		bool mScaleNormals = false;
};

#endif // NormalSphereSELECTIONRenderWIDGET_H
