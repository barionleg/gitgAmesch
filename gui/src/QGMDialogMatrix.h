#ifndef QGMDIALOGMATRIX_H
#define QGMDIALOGMATRIX_H

#include <QDialog>

namespace Ui {
	class QGMDialogMatrix;
}

class QGMDialogMatrix : public QDialog
{
		Q_OBJECT

	public:
		explicit QGMDialogMatrix(QWidget *parent = nullptr);
		~QGMDialogMatrix();

	private:
		Ui::QGMDialogMatrix *ui;
};

#endif // QGMDIALOGMATRIX_H
