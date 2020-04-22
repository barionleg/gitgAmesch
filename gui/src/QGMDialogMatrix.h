//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

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

	private slots:
		void updateMatrixValues();
		void tabChanged(int index);
		void updateScale(double value);
		void updateTranslate();
		void updateRotate(double angle);
		void updateRotateBySlider(int value);
		void resetValues();

	private:
		Ui::QGMDialogMatrix *ui;

		//mMatrixData stores the matrix in column-major order
		std::array<double,16> mMatrixData = {1.0,0.0,0.0,0.0,
		                                     0.0,1.0,0.0,0.0,
		                                     0.0,0.0,1.0,0.0,
		                                     0.0,0.0,0.0,1.0};
		std::array<QLineEdit*,16> mLineEditPtrs;

		QDoubleValidator mValidator;



};

#endif // QGMDIALOGMATRIX_H
