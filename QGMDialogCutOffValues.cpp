#include "QGMDialogCutOffValues.h"

QGMDialogCutOffValues::QGMDialogCutOffValues( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	//! Constructor
	setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}

void QGMDialogCutOffValues::accept() {
	//! Checks if all the input values are correct. If the values seem to be correct
	//! a signal is emitted, which should be connected MeshQt.
	//cout << "[QGMDialogCutOffValues::accept] lineVertex: " << lineRadius->text().toStdString() << endl;

	bool  convertOk;
	bool  doNotAccept = false;

	double minVal = lineMin->text().toDouble( &convertOk );
	if( !convertOk ){
		QSETCOLOR( lineMin, 255, 128, 128 ); // light read
		doNotAccept = true;
	} else {
		QSETCOLOR( lineMin, 255, 255, 255 ); // white
	}
	//cout << "[QGMDialogCutOffValues::accept] lineMin: " << minVal << endl;

	double maxVal = lineMax->text().toDouble( &convertOk );
	if( !convertOk ) {
		QSETCOLOR( lineMax, 255, 128, 128 ); // light read
		doNotAccept = true;
	} else {
		QSETCOLOR( lineMax, 255, 255, 255 ); // white
	}
	//cout << "[QGMDialogCutOffValues::accept] lineMax: " << maxVal << endl;

	if( doNotAccept ) {
		return;
	}

	emit cutOffValues( minVal, maxVal, checkNAN->isChecked() );

	//! Disconnects all connections - so other other objectrrs can reuse an instance of this class.
	QObject::disconnect();
	QDialog::accept();
}