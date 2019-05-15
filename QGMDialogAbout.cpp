#include "QGMDialogAbout.h"

QGMDialogAbout::QGMDialogAbout( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	//! Constructor
	setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}
