#ifndef PINRENDERER_H
#define PINRENDERER_H

#include "meshGL.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix>

class PinRenderer
{
public:

	struct PinVertex {
		GLfloat  position[3]; //!< xyz-coordinate.
		GLfloat  normal[3];   //!< Normal vector.
		GLubyte  color[3];    //!< Color information (RGBA).
	};

	PinRenderer();
	~PinRenderer();

	//prevent copy and moving
	PinRenderer(const PinRenderer& other) = delete;
	PinRenderer& operator=(const PinRenderer& other) = delete;

	PinRenderer(const PinRenderer&& other) = delete;
	PinRenderer& operator=(const PinRenderer&& other) = delete;

	//call these with valid OpenGL context
	bool init();
	void destroy();
	void render(std::vector<PinVertex> &pinPoints, const QMatrix4x4 &projectionMatrix, const QMatrix4x4 &modelViewMatrix, float pinSize);


	bool isInitialized();
private:

	bool initializeShader();
	bool initializePinGeometry();

	bool resizePinPointsBuffer(unsigned int size);

	bool mIsInitialized;
	unsigned int mAllocatedBufferSize;

	unsigned int mPinFaceIndicesSize;

	QOpenGLFunctions_3_3_Core mGL;
	QOpenGLShaderProgram* mShaderPins = nullptr;

	QOpenGLBuffer mPinPositionBuffer;
	QOpenGLBuffer mPinNormalBuffer;
	QOpenGLBuffer mPinHeadFlags;
	QOpenGLBuffer mPinFaceIndexBuffer;
	QOpenGLBuffer mInstancedPinBuffer;
	QOpenGLVertexArrayObject mVAO;
};

#endif // PINRENDERER_H
