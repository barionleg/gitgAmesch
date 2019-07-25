#include "QGMDialogSlider.h"

#define QGMDIALOGSLIDERDEFAULTS \
	mIndex( -1 ),           \
	mInitialValue( 0.5 ),   \
	mMinVal( 0.0 ),         \
	mMaxVal( 1.0 ),         \
	mFactor( 1.0 ),         \
	mLogarithmic( false )

QGMDialogSlider::QGMDialogSlider( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ), QGMDIALOGSLIDERDEFAULTS {
	//! Constructor
	setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );

	// Setup validators and min/max
	mValidRel.setBottom( 0.0 );
	mValidRel.setTop( 1.0 );
	lineValueRel->setValidator( &mValidRel );
	lineValueAbs->setValidator( &mValidAbs );
	setMin( mMinVal );
	setMax( mMaxVal );

	// Setup signals
	QObject::connect( horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int))    );
	QObject::connect( checkBoxPreview,  SIGNAL(toggled(bool)),     this, SLOT(previewChecked(bool)) );

	QObject::connect( lineValueRel,     SIGNAL(textEdited(QString)), this, SLOT(valueChangedRel(QString)) );
	QObject::connect( lineValueAbs,     SIGNAL(textEdited(QString)), this, SLOT(valueChangedAbs(QString)) );
}

// --- Methods -------------------------------------------------------------------------

int QGMDialogSlider::setIdx( int setIdx ) {
	//! Set an index to be sent with a signal.
	//! Returns the old value
	int tmpVal = mMinVal;
	mIndex = setIdx;
	return tmpVal;
}

double QGMDialogSlider::setMin( double setMinVal ) {
	//! Sets the minimum value of the slider.
	//! Returns the old value.
	setMinVal *= mFactor;
	double tmpVal = mMinVal;
	if( horizontalSlider->invertedAppearance() ) {
		mMaxVal = setMinVal;
	} else {
		mMinVal = setMinVal;
	}
	mValidAbs.setBottom( setMinVal );
	lineValueMin->setText( tr( "%1" ).arg( setMinVal ) );
	return tmpVal;
}

double QGMDialogSlider::setMax( double setMaxVal ) {
	//! Sets the maximum value of the slider.
	//! Returns the old value.
	setMaxVal *= mFactor;
	double tmpVal = mMaxVal;
	if( horizontalSlider->invertedAppearance() ) {
		mMinVal = setMaxVal;
	} else {
		mMaxVal = setMaxVal;
	}
	mValidAbs.setTop( setMaxVal );
	lineValueMax->setText( tr( "%1" ).arg( setMaxVal ) );
	return tmpVal;
}

int QGMDialogSlider::setSteps( int rMaxSteps ) {
	//! Sets the maximum number of steps.
	//! Returns the old value.
	int tmpVal = horizontalSlider->maximum();
	horizontalSlider->setMaximum( rMaxSteps );
	return tmpVal;
}

double QGMDialogSlider::setPos( double setPos ) {
	//! Sets the inital value of the slider.
	//! ATTENTION: Has to be called AFTER setMin and setMax
	//! Returns the old value.
	setPos *= mFactor;
	if( mLogarithmic ) {
		setPos = log( setPos );
	}
	double tmpVal = mInitialValue;
	mInitialValue = setPos;

	int sliderPos = ( ( ( setPos - mMinVal ) / ( mMaxVal - mMinVal ) ) * ( (double)horizontalSlider->maximum() - (double)horizontalSlider->minimum() ) ) + horizontalSlider->minimum();
	horizontalSlider->setSliderPosition( sliderPos );
	valueChanged( sliderPos );

	return tmpVal;
}

//! Sets a multiplier.
//! Attention: Has to be called BEFORE setPos, setMin and setMax
//! Returns the old value.
double QGMDialogSlider::setFactor( double rFactor ) {
	double tmpVal = mFactor;
	mFactor = rFactor;
	return tmpVal;
}

bool QGMDialogSlider::setInverted( bool setTo ) {
	//! Inverts the ruler e.g. for selection of values "bigger than".
	//! Returns the old state.
	//! Remarks: will be set to false, when accept() is called.
	//! Should be called before setting min and max.
	bool tmpSet = horizontalSlider->invertedAppearance();
	horizontalSlider->setInvertedAppearance( setTo );
	return tmpSet;
}

//! Return logaritihmic instead of linear values.
bool QGMDialogSlider::setLogarithmic( bool rSetTo ) {
	mLogarithmic = rSetTo;
	return true;
}

void QGMDialogSlider::suppressPreview() {
	//! Disables the preview checkbox until accept or reject are called.
	//! Usefull, when called for a function having no preview capability.
	checkBoxPreview->setChecked( false );
	checkBoxPreview->setDisabled( true );
}

// --- Value Access --------------------------------------------------------------------------------------------------------------------------------------------

//! Get the selected value.
//! @returns false in case of an error.
bool QGMDialogSlider::getValue( double* rValue ) {
	int sliderPos = horizontalSlider->sliderPosition();
	double valRel = ( (double)sliderPos - (double)horizontalSlider->minimum() ) / ( (double)horizontalSlider->maximum() - (double)horizontalSlider->minimum() );
	double valSel = ( mMaxVal - mMinVal ) * valRel + mMinVal;
	if( mLogarithmic ) {
		valSel = exp( valSel );
	}
	valSel /= mFactor;
	(*rValue) = valSel;
	return true;
}

// --- SLOTS (private) -----------------------------------------------------------------------------------------------------------------------------------------

//! Sends an initial preview signal - see: valuePreview
void QGMDialogSlider::previewChecked( bool state ) {
	if( !state ) {
		return;
	}
	int sliderPos = horizontalSlider->sliderPosition();
	valueChanged( sliderPos );
}

//! Accepts a double value as string, which is assumed to be checked by a QDoubleValidator before.
void QGMDialogSlider::valueChangedRel( QString rValRel ) {
	// BROKEN thanks to some locale mixup: hasAcceptableInput 
	//if( !lineValueRel->hasAcceptableInput() ) {
	//	cerr << "[QGMDialogSlider::" << __FUNCTION__ << "] ERROR: NOT hasAcceptableInput!" << endl;
	//	QSETCOLOR( lineValueRel, 255, 128, 128 );
	//	return;
	//}
	QSETCOLOR( lineValueRel, 255, 255, 255 );
	QSETCOLOR( lineValueAbs, 255, 255, 255 );
	bool   convertOk;
	double valRel = rValRel.toDouble( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMDialogSlider::" << __FUNCTION__ << "] ERROR: convert false!" << endl;
		return;
	}
	int sliderPos = valRel * ( horizontalSlider->maximum() - horizontalSlider->minimum() ) + horizontalSlider->minimum();
	QObject::disconnect( horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int))    );
	QObject::disconnect( lineValueAbs,     SIGNAL(textEdited(QString)), this, SLOT(valueChangedAbs(QString)) );
	horizontalSlider->setSliderPosition( sliderPos );
	double valPre = ( mMaxVal - mMinVal ) * valRel + mMinVal;
	if( mLogarithmic ) {
		valPre = exp( valPre );
	}
	lineValueAbs->setText( tr( "%1" ).arg( valPre ) );
	QObject::connect( horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int))    );
	QObject::connect( lineValueAbs,     SIGNAL(textEdited(QString)), this, SLOT(valueChangedAbs(QString)) );
	if( !checkBoxPreview->isChecked() ) {
		return;
	}
	// Take care about preview:
	if( !checkBoxPreview->isChecked() ) {
		return;
	}
	emit valuePreviewInt( (int)round( valPre ) );
	emit valuePreviewFloat( valPre );
	emit valuePreviewIdFloat( mIndex, valPre );
}

//! Accepts a double value as string, which is assumed to be checked by a QDoubleValidator before.
void QGMDialogSlider::valueChangedAbs( QString rValAbs ) {
	// BROKEN thanks to some locale mixup: hasAcceptableInput 
	//if( !lineValueAbs->hasAcceptableInput() ) {
	//	cerr << "[QGMDialogSlider::" << __FUNCTION__ << "] ERROR: NOT hasAcceptableInput!" << endl;
	//	QSETCOLOR( lineValueAbs, 255, 128, 128 );
	//	return;
	//}
	QSETCOLOR( lineValueRel, 255, 255, 255 );
	QSETCOLOR( lineValueAbs, 255, 255, 255 );
	bool   convertOk;
	double valAbs = rValAbs.toDouble( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMDialogSlider::" << __FUNCTION__ << "] ERROR: convert false!" << endl;
		return;
	}
	if( mLogarithmic ) {
		valAbs = log( valAbs );
	}
	double valRel = ( valAbs - mMinVal ) / ( mMaxVal - mMinVal );
	int sliderPos = valRel * ( horizontalSlider->maximum() - horizontalSlider->minimum() ) + horizontalSlider->minimum();
	QObject::disconnect( horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int))    );
	QObject::disconnect( lineValueRel,     SIGNAL(textEdited(QString)), this, SLOT(valueChangedRel(QString)) );
	horizontalSlider->setSliderPosition( sliderPos );
	lineValueRel->setText( tr( "%1" ).arg( valRel ) );
	QObject::connect( horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int))    );
	QObject::connect( lineValueRel,     SIGNAL(textEdited(QString)), this, SLOT(valueChangedRel(QString)) );
	// Take care about preview:
	if( !checkBoxPreview->isChecked() ) {
		return;
	}
	double valPre = ( mMaxVal - mMinVal ) * valRel + mMinVal;
	if( mLogarithmic ) {
		valPre = exp( valPre );
	}
	emit valuePreviewInt( (int)round( valPre ) );
	emit valuePreviewFloat( valPre );
	emit valuePreviewIdFloat( mIndex, valPre );
}

//! Checks if preview is activated.
//! If so signals with selected values of the slider are emitted.
void QGMDialogSlider::valueChanged( int sliderPos ) {
	double valRel = ( (double)sliderPos - (double)horizontalSlider->minimum() ) / ( (double)horizontalSlider->maximum() - (double)horizontalSlider->minimum() );
    double valPre = ( mMaxVal - mMinVal ) * valRel + mMinVal;
	if( mLogarithmic ) {
		valPre = exp( valPre );
	}
	lineValueAbs->setText( tr( "%1" ).arg( valPre ) );
	lineValueRel->setText( tr( "%1" ).arg( valRel ) );
	QSETCOLOR( lineValueRel, 255, 255, 255 );
	QSETCOLOR( lineValueAbs, 255, 255, 255 );
	if( !checkBoxPreview->isChecked() ) {
		return;
    }
	emit valuePreviewInt( (int)round( valPre ) );
	emit valuePreviewFloat( valPre/mFactor );
	emit valuePreviewIdFloat( mIndex, valPre/mFactor );
	//cout << __PRETTY_FUNCTION__ << " valPre: " << valPre << endl;
}

// --- SLOTS (public) --------------------------------------------------------------------

//! Checks if all the input values are correct. If the values seem to be correct
//! a signal is emitted, which should be connected MeshQt.
void QGMDialogSlider::accept() {
	int sliderPos = horizontalSlider->sliderPosition();
	double valRel = ( (double)sliderPos - (double)horizontalSlider->minimum() ) / ( (double)horizontalSlider->maximum() - (double)horizontalSlider->minimum() );
	double valSel = ( mMaxVal - mMinVal ) * valRel + mMinVal;
	if( mLogarithmic ) {
		valSel = exp( valSel );
	}
	//cout << "[QGMDialogSlider::" << __FUNCTION__ << "] valSel: " << valSel << endl;

	hide();

	emit valueSelected( (int)round( valSel ) );
	emit valueSelected( valSel );
	emit valueSelected( mIndex, valSel );

	// set to default:
	checkBoxPreview->setEnabled( true );
	horizontalSlider->setInvertedAppearance( false );
	// to prevent multiple and wrong connections:
	QObject::disconnect();

	QDialog::accept();
}

//! Closes the dialog and emits the initial value in case preview has changed things.
void QGMDialogSlider::reject() {
    if(mLogarithmic)
    {
        mInitialValue = exp( mInitialValue );
    }

	emit valuePreviewInt( mIndex );
	emit valuePreviewFloat( mInitialValue  );
	emit valuePreviewIdFloat( mIndex, mInitialValue  );

	// set to default:
	checkBoxPreview->setEnabled( true );
	horizontalSlider->setInvertedAppearance( false );
	// to prevent multiple and wrong connections:
	QObject::disconnect();

	QDialog::reject();
}
