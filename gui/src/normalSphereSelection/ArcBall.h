#ifndef ARCBALL_H
#define ARCBALL_H

#include <QQuaternion>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>

class ArcBall
{
	public:
		ArcBall(const QVector3D& center = QVector3D(0.0,0.0,0.0), float radius = 2.0);

		~ArcBall() = default;

		void beginDrag(const QVector2D& screenCoords);

		void drag(const QVector2D& screenCoords);

		[[nodiscard]] QQuaternion getTransformationQuat() const;
		void setTransformationQuat(const QQuaternion& quat);
	private:

		QVector3D mouseOnSphere(const QVector2D& mousePos);
		static QQuaternion quatFromUnitSphere(const QVector3D& from, const QVector3D& to);

		QVector3D mCenter;
		float mRadius;

		QQuaternion mQNow;
		QQuaternion mQDown;

		QVector3D mMouseStart;
};

#endif // ARCBALL_H
