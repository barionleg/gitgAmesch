#include "QGMDialogMatrix.h"
#include "ui_QGMDialogMatrix.h"

QGMDialogMatrix::QGMDialogMatrix(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QGMDialogMatrix)
{
	ui->setupUi(this);
}

QGMDialogMatrix::~QGMDialogMatrix()
{
	delete ui;
}
