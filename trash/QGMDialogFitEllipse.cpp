#include "QGMDialogFitEllipse.h"

QGMDialogFitEllipse::QGMDialogFitEllipse(QWidget *parent) :
	QDialog(parent), ui(new Ui::QGMDialogFitEllipse) {
	//! Constructor
	ui->setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	// Set default parameters
	spinPlains = 2;
	spinSteps  = 3.6;
}

QGMDialogFitEllipse::~QGMDialogFitEllipse()
{
    delete ui;
}

void QGMDialogFitEllipse::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void QGMDialogFitEllipse::on_buttonBox_accepted()
{
    emit startEllipseFit( this );
}

void QGMDialogFitEllipse::on_planesSpinBox_valueChanged(int plains)
{
    spinPlains=plains;
    //    cout << "plains:" << plains << endl;
    emit plainsEllipseFit( this );
}

void QGMDialogFitEllipse::on_stepSpinBox_valueChanged(double steps)
{
    spinSteps=steps;
    //    cout << "step:" << steps << endl;
    emit stepsEllipseFit( this );
}

void QGMDialogFitEllipse::on_buttonBox_rejected()
{
    emit clearScreenFromEllipse(this);
}
