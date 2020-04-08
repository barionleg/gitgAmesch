#ifndef QGMDIALOGMATRIX_H
#define QGMDIALOGMATRIX_H

#include <QDialog>
#include <QDoubleValidator>
#include <array>
#include <memory>

class QLineEdit;

namespace Ui {
	class QGMDialogMatrix;
}

class QGMDialogMatrix : public QDialog
{
		Q_OBJECT

	public:
		explicit QGMDialogMatrix(QWidget *parent = nullptr);
		~QGMDialogMatrix();

		void getValues(std::vector<double>& values) const;

	public slots:
		void copyToClipboard() const;
		void fetchClipboard( );

	private:
		Ui::QGMDialogMatrix *ui;

		//mMatrixData stores the matrix in column-major order
		std::array<double,16> mMatrixData = {1.0,0.0,0.0,0.0,
		                                     0.0,1.0,0.0,0.0,
		                                     0.0,0.0,1.0,0.0,
		                                     0.0,0.0,0.0,1.0};
		std::array<QLineEdit*,16> mLineEditPtrs;

		QDoubleValidator mValidator;
	private slots:
		void updateMatrixValues();
		void tabChanged(int index);
		void updateScale(double value);
		void updateTranslate();
		void updateRotate(double angle);
		void updateRotateBySlider(int value);
		void resetValues();
};

#endif // QGMDIALOGMATRIX_H
