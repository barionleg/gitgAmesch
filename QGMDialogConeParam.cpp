/*!
*	@file   QGMDialogConeParam.cpp
*	@brief  Logic for the cone parameter dialog
*/

#include <limits>
#include <QMessageBox>
#include "QGMDialogConeParam.h"

QGMDialogConeParam::QGMDialogConeParam(QWidget* parent, Qt::WindowFlags flags) 
: QDialog(parent, flags) {
	setupUi(this);
	QIcon gigaMeshLogo(_GIGAMESH_LOGO_);
	setWindowIcon(gigaMeshLogo);

	QAbstractButton* applyButton = this->buttonBox->button( QDialogButtonBox::Apply );
	QObject::connect( applyButton, &QAbstractButton::clicked, this, &QGMDialogConeParam::apply  );
	QObject::connect( buttonBox, &QDialogButtonBox::accepted, this, &QGMDialogConeParam::accept );
	QObject::connect( buttonBox, &QDialogButtonBox::rejected, this, &QGMDialogConeParam::reject );
}

//! Called when dialog is shown; stores current values of dialog in the clipboard
//! to simplify user interaction.
void QGMDialogConeParam::showEvent( QShowEvent* event ) {
	// Check clipboard
	QClipboard* clipboard = QApplication::clipboard();
	QString currText = clipboard->text();
	// Map line breaks and tabs to space
	currText.replace( QChar('\n'), QString(" ") );
	currText.replace( QChar('\r'), QString(" ") );
	currText.replace( QChar('\0'), QString(" ") );
	currText.replace( QChar('\t'), QString(" ") );
	// Remove extra white space
	currText = currText.trimmed();
	int strLenBefore = -1;
	while( currText.length() != strLenBefore ) {
		strLenBefore = currText.length();
		currText.replace( QString("  "), QString(" ") );
	}

	// Try to parse values.
	std::istringstream converter( currText.toStdString() );
	double x1, y1, z1, x2, y2, z2, r1, r2;
	converter >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> r1 >> r2;
	if( converter ) {
		Vector3D upperAxis( x1, y1, z1, 1.0 );
		Vector3D lowerAxis( x2, y2, z2, 1.0 );
		bool userAnswer;
		bool userCancel;
		SHOW_QUESTION( "Use clipboard data",
		               "Do you want to use the values from the clipboard to define a cone?"
		               "<br /><br />" + currText,
		               userAnswer, userCancel );
		if( userCancel ) {
			return;
		}
		if( userAnswer ) {
			this->setAxis( upperAxis, lowerAxis );
			this->setRadii( r1, r2 );
		}
	}
}

//! Sets upper and lower axis vector.
void QGMDialogConeParam::setAxis(const Vector3D &upperAxis, const Vector3D &lowerAxis) {

	x1->setText(tr("%1").arg(upperAxis.getX()));
	y1->setText(tr("%1").arg(upperAxis.getY()));
	z1->setText(tr("%1").arg(upperAxis.getZ()));
	x2->setText(tr("%1").arg(lowerAxis.getX()));
	y2->setText(tr("%1").arg(lowerAxis.getY()));
	z2->setText(tr("%1").arg(lowerAxis.getZ()));

}

//! Sets upper and lower radius.
void QGMDialogConeParam::setRadii(double upperRadius, double lowerRadius) {

	this->upperRadius->setText( tr("%1").arg(upperRadius));
	this->lowerRadius->setText( tr("%1").arg(lowerRadius));

}

//! Accepts currently selected parameters
void QGMDialogConeParam::accept() {

	Vector3D axisTop;
	Vector3D axisBot;

	double upperRadius = 0.0;
	double lowerRadius = 0.0;

	// Check input data for correctness
	if(this->checkData(axisTop, axisBot, upperRadius, lowerRadius)) {
		this->copyDataToClipboard();
		emit coneParameters(axisTop, axisBot, upperRadius, lowerRadius);
		QDialog::accept();
	}
	else
	{
		QDialog::reject();
		return;
	}
}

//! Applies currently selected parameters without closing the dialog window. This is
//! handy for changes to the cone in quick succession.
void QGMDialogConeParam::apply() {

	Vector3D axisTop;
	Vector3D axisBot;

	double upperRadius = 0.0;
	double lowerRadius = 0.0;

	// Check input data for correctness
	if(this->checkData(axisTop, axisBot, upperRadius, lowerRadius)) {
		this->copyDataToClipboard();
		emit coneParameters(axisTop, axisBot, upperRadius, lowerRadius);
	}
	else {
		return;
	}
}

//! Checks whether the current data entered by the user is valid or not. This is used
//! for the `accept()` and `apply()` functions.
//! @param _axisTop     Reference to store axis vector (top)
//! @param _axisBot     Reference to store axis vector (bottom)
//! @param _upperRadius Reference to store upper radius
//! @param _lowerRadius Reference to store lower radius
//! @note Upon success, all referenced parameters will be overwritten with the correctly
//!       parsed values.
//! @returns true if all values could be parsed, else false.
bool QGMDialogConeParam::checkData(Vector3D& _axisTop, Vector3D& _axisBot, double& _upperRadius, double& _lowerRadius) {

	bool invalidValues = false;
	bool conversionOK  = false;

	// Batch convert all text fields to coordinates

	QLineEdit* lineEdits[] = { x1,
	                           y1,
	                           z1,
	                           x2,
	                           y2,
	                           z2 };
	double values[3] = {0, 0, 0};

	for(int i = 0; i < 6; i++) {

		values[i % 3] = lineEdits[i]->text().toDouble(&conversionOK);
		if(!conversionOK) {
			invalidValues = true;
			break;
		}
		else if(i == 2) {
			_axisTop.set(values[0], values[1], values[2]);
		}
		else if(i == 5) {
			_axisBot.set(values[0], values[1], values[2]);
		}
	}

	// Convert radii

	double r0      = upperRadius->text().toDouble(&conversionOK);
	invalidValues |= !conversionOK;
	if(conversionOK) {
		_upperRadius = r0; // only overwrite values if conversion worked
	}

	double r1      = lowerRadius->text().toDouble(&conversionOK);
	invalidValues |= !conversionOK;
	if(conversionOK) {
		_lowerRadius = r1; // only overwrite values if conversion worked
	}

	return(!invalidValues);
}

//! Copies current data of the cone dialog to the clipboard. This may be called each
//! time the dialog parameters change.
void QGMDialogConeParam::copyDataToClipboard() {

	QClipboard* clipboard = QApplication::clipboard();
	QString newText =   x1->text() + " " + y1->text() + " " + z1->text() + "\n"
			  + x2->text() + " " + y2->text() + " " + z2->text() + "\n"
			  + upperRadius->text() + "\n"
			  + lowerRadius->text() + "\n";

	clipboard->setText(newText);
}
