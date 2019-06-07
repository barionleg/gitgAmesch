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
		~NormalSphereSelectionRenderWidget() override = default;

		//moves the data from normals to internal data => normals is afterward empty!
		void setRenderNormals(std::vector<float>& normals);

		void setSelected(float nx, float ny, float nz);
		void clearSelected();
		bool isNormalSelected(float nx, float ny, float nz);
		// QWidget interface
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
		QMatrix4x4 mViewMatrix;
		QMatrix4x4 mProjectionMatrix;

		QOpenGLTexture mFuncValTexture;

		QOpenGLBuffer mIcosphereBuffer;
		QOpenGLBuffer mIcosphereDataBuffer;
		QOpenGLBuffer mIcosphereIndices;
		QOpenGLVertexArrayObject mIcoSphereVAO;
		QOpenGLShaderProgram mIcoSphereShader;

		bool mUpdateSelectionTexture;

		std::vector<float> mNormalUpload;
		GLsizei mNumNormals;

		float mMaxNormalDensity;

		ArcBall mArcBall;

		int mScreenWidth;
		int mScreenHeight;

		IcoSphereTree mIcoSphereTree;
		float mMinData;
};

#endif // NormalSphereSELECTIONRenderWIDGET_H
