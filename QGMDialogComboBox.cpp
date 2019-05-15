#include "QGMDialogComboBox.h"

//! Constructor
QGMDialogComboBox::QGMDialogComboBox( QWidget *parent, Qt::WindowFlags flags ) :
    QDialog( parent, flags ) {
	// Init UI elements:
	setupUi( this );
	// Set logo:
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}

//! Set the decriptive text label for the combo-box.
void QGMDialogComboBox::setTextLabel( const QString & rLabelText ) {
	mLabelText->setText( rLabelText );
}

//! Adds an entry to the combo-box.
void QGMDialogComboBox::addItem( const QString & rText, const QVariant & rUserData ) {
	mComboBox->addItem( rText, rUserData );
}

//! Returns the selected item. E.g. the value of an enumerator.
QVariant QGMDialogComboBox::getSelectedItem() {
	int index = mComboBox->currentIndex();
	QVariant userData = mComboBox->itemData ( index );
	return userData;
}
