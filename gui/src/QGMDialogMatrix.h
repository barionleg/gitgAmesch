#ifndef QGMDIALOGMATRIX_H
#define QGMDIALOGMATRIX_H

#include <QDialog>
#include <array>

class QLineEdit;
class QDoubleValidator;

namespace Ui {
	class QGMDialogMatrix;
}

class QGMDialogMatrix : public QDialog
{
		Q_OBJECT

	public:
		explicit QGMDialogMatrix(QWidget *parent = nullptr);
		~QGMDialogMatrix();

		void fetchClipboard( );

		void getValues(std::vector<double>& values) const;

	private:
		Ui::QGMDialogMatrix *ui;

		std::array<double,16> mMatrixData;
		std::array<QLineEdit*,16> mLineEditPtrs;

		std::unique_ptr<QDoubleValidator> mValidator;
	private slots:
		void updateMatrixValues();
};

#endif // QGMDIALOGMATRIX_H
