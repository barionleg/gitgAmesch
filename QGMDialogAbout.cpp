#include "QGMDialogAbout.h"

QGMDialogAbout::QGMDialogAbout( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	//! Constructor
	setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}


void QGMDialogAbout::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
