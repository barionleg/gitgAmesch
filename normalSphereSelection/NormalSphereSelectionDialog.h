#ifndef NormalSphereSELECTIONDIALOG_H
#define NormalSphereSELECTIONDIALOG_H

#include <QDialog>
#include <QQuaternion>
#include "vector3d.h"

class QAbstractButton;

namespace Ui {
	class NormalSphereSelectionDialog;
}

class MeshQt;

class NormalSphereSelectionDialog : public QDialog
{
		Q_OBJECT

	public:
		explicit NormalSphereSelectionDialog(QWidget *parent = nullptr);
		~NormalSphereSelectionDialog() override;

		void setMeshNormals(MeshQt* mesh);
		void selectMeshByNormals();

	signals:
		void rotationChanged(QQuaternion);

	private slots:
		void comboButtonBoxClicked(QAbstractButton* button);
		void renderWidgetRotationChanged(QQuaternion quat);
	public slots:
		void updateRotationExternal(Vector3D camCenter, Vector3D camUp);
	private:
		Ui::NormalSphereSelectionDialog *ui;

		MeshQt* mMesh;
};

#endif // NormalSphereSELECTIONDIALOG_H
