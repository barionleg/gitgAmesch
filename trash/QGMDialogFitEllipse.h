#ifndef QGMDIALOGFITELLIPSE_H
#define QGMDIALOGFITELLIPSE_H

#include <QDialog>

// Qt Interface includes:
#include "ui_dialogFitEllipse.h"

#include "QGMMacros.h"

namespace Ui {
    class QGMDialogFitEllipse;
}

class QGMDialogFitEllipse : public QDialog {
    Q_OBJECT
public:
    QGMDialogFitEllipse(QWidget *parent = 0);
    ~QGMDialogFitEllipse();
    int spinPlains;
    double spinSteps;
protected:
    void changeEvent(QEvent *e);

private:
    Ui::QGMDialogFitEllipse *ui;

signals:
    void startEllipseFit(QGMDialogFitEllipse*); //!< emitted when this dialog is successfully closed - see: MeshQt::startEllipseFit and accept()
    void stepsEllipseFit(QGMDialogFitEllipse*); //!< emitted when this dialog is successfully closed - see: MeshQt::stepsEllipseFit and accept()
    void plainsEllipseFit(QGMDialogFitEllipse*); //!< emitted when this dialog is successfully closed - see: MeshQt::plainsEllipseFit and accept()
    void clearScreenFromEllipse(QGMDialogFitEllipse*); //!< emitted when this dialog is successfully closed - see: MeshQt::plainsEllipseFit and accept()


private slots:
    void on_buttonBox_rejected();
    void on_stepSpinBox_valueChanged(double );
    void on_planesSpinBox_valueChanged(int );
    void on_buttonBox_accepted();
};

#endif // QGMDIALOGFITELLIPSE_H
