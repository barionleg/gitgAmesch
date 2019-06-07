/*!
*	@file  QGMDialogConeParam.h
*	@brief Describes cone parameter dialog
*/

#ifndef QGMDIALOGCONEPARAM_H
#define QGMDIALOGCONEPARAM_H

#include <QtGui>

#if QT_VERSION >= 0x050000
#include <QAbstractButton>
#include <QPushButton>
#endif

#include <QIcon>
#include <QDialog>

#include "vector3d.h"

#include "ui_dialogConeParam.h"
#include "QGMMacros.h"

class QGMDialogConeParam : public QDialog, private Ui::dialogConeParam {
	Q_OBJECT

public:
	QGMDialogConeParam(QWidget* parent = 0, Qt::WindowFlags flags = 0);

	void setAxis(const Vector3D& upperAxis, const Vector3D& lowerAxis);
	void setRadii(double upperRadius, double lowerRadius);

	void accept();
        void showEvent(QShowEvent*);

signals:
	void coneParameters(const Vector3D&, const Vector3D&, double, double);

private:
	void copyDataToClipboard();
	bool checkData(Vector3D& axisTop, Vector3D& axisBot, double& upperRadius, double& lowerRadius);

private slots:
	void apply();
};

#endif // QGMDIALOGCONEPARAM_H
