#ifndef NormalSphereSELECTIONDIALOG_H
#define NormalSphereSELECTIONDIALOG_H

#include <QDialog>

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
	private slots:
		void comboButtonBoxClicked(QAbstractButton* button);


	private:
		Ui::NormalSphereSelectionDialog *ui;

		MeshQt* mMesh;
};

#endif // NormalSphereSELECTIONDIALOG_H
