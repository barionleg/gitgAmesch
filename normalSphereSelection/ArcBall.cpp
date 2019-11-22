#include "ArcBall.h"

#include <QtMath>

ArcBall::ArcBall(const QVector3D& center, float radius) : mCenter(center), mRadius(radius), mMouseStart(0.0F,0.0F,0.0F)
{

}

void ArcBall::beginDrag(const QVector2D& screenCoords)
{
	mQDown = mQNow;
	mMouseStart = mouseOnSphere(screenCoords);
}

void ArcBall::drag(const QVector2D& screenCoords)
{
	QVector3D currentMousePos = mouseOnSphere(screenCoords);

	QQuaternion qDrag = quatFromUnitSphere(mMouseStart, currentMousePos);

	mQNow = qDrag * mQDown;
}

QQuaternion ArcBall::getTransformationQuat() const
{
	return mQNow;
}

void ArcBall::setTransformationQuat(const QQuaternion& quat)
{
	mQNow = quat;
}

QVector3D ArcBall::mouseOnSphere(const QVector2D& mousePos)
{
	QVector3D ballMouse;

	ballMouse.setX( (mousePos.x() - mCenter.x()) / mRadius );
	ballMouse.setY( (mousePos.y() - mCenter.y()) / mRadius );

	float mag = QVector3D::dotProduct(ballMouse, ballMouse);	//mag == squared length == dot(x,x)

	if(mag > 1.0)
	{
		ballMouse *= 1.0F / sqrtf(mag);
		ballMouse.setZ(0.0);
	}

	else {
		ballMouse.setZ(sqrtf(1.0F - mag));
	}

	return ballMouse;
}

QQuaternion ArcBall::quatFromUnitSphere(const QVector3D& from, const QVector3D& to)
{
	QQuaternion q;
	q.setVector(QVector3D::crossProduct(from, to));
	q.setScalar(QVector3D::dotProduct(from, to));
	return q;
}
