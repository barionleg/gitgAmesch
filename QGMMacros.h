#ifndef QGMMACROS_H
#define QGMMACROS_H

#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QTimer>
#include "QGMDialogComboBox.h"

//! Helpfull stuff to prevent copy & paste for changing the GUI. 

inline void QSETCOLOR(QWidget* someqobject, unsigned char r, unsigned char g, unsigned char b ) {
	QPalette p = someqobject->palette();
	p.setColor( QPalette::Base, QColor( r, g, b ) );
	someqobject->setPalette( p );
}

#define _GIGAMESH_LOGO_ ":/GMGeneric/GigaMesh_Logo.png"

inline void SHOW_MSGBOX_INFO( const QString& rTextShort, const QString& rTextLong ) {
	QMessageBox msgBox;
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	msgBox.setWindowTitle( rTextShort );
	msgBox.setText( rTextShort );
	msgBox.setIcon( QMessageBox::Information );
	msgBox.setInformativeText( rTextLong );
	msgBox.setStandardButtons( QMessageBox::Ok );
	msgBox.exec();
}

template<class T, class U>
inline void SHOW_MSGBOX_INFO_SAVE( const QString& rTextShort, const QString& rTextLong, const T& rTargetObject, const U& rTargetSlot ) {
	QMessageBox msgBox;
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	msgBox.setWindowTitle( rTextShort );
	msgBox.setText( rTextShort );
	msgBox.setIcon( QMessageBox::Information );
	msgBox.setInformativeText( rTextLong );
	msgBox.setStandardButtons( QMessageBox::Ok );
	QPushButton* saveButtonPtr = msgBox.addButton( QMessageBox::Save );
	QObject::connect( saveButtonPtr, &QPushButton::clicked, rTargetObject, rTargetSlot );
	msgBox.exec();
}

inline void SHOW_MSGBOX_WARN( const QString& rTextShort, const QString& rTextLong ) {
	QMessageBox msgBox;
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	msgBox.setWindowTitle( rTextShort );
	msgBox.setText( rTextShort );
	msgBox.setIcon( QMessageBox::Warning );
	msgBox.setInformativeText( rTextLong );
	msgBox.setStandardButtons( QMessageBox::Ok );
	msgBox.exec();
}

inline void SHOW_MSGBOX_WARN_TIMEOUT( const QString& rTextShort, const QString& rTextLong, int rTimeOut ) {
	QMessageBox *msgBox = new QMessageBox;
	msgBox->setAttribute( Qt::WA_DeleteOnClose, true );
	msgBox->setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	msgBox->setWindowTitle( rTextShort );
	msgBox->setText( rTextShort );
	msgBox->setIcon( QMessageBox::Warning );
	msgBox->setInformativeText( rTextLong );
	msgBox->setStandardButtons( QMessageBox::Ok );
	msgBox->show();
	QTimer::singleShot( rTimeOut, msgBox, SLOT(close()) );
}

inline void SHOW_MSGBOX_CRIT( const QString& rTextShort, const QString& rTextLong ) {
	QMessageBox msgBox;
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	msgBox.setWindowTitle( rTextShort );
	msgBox.setText( rTextShort );
	msgBox.setIcon( QMessageBox::Critical );
	msgBox.setInformativeText( rTextLong );
	msgBox.setStandardButtons( QMessageBox::Ok );
	msgBox.exec();
}

inline void SHOW_QUESTION( const QString& rTextShort, const QString& rTextLong, bool& rAnswer, bool& rCancel ) {
	QMessageBox msgBox;
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
	msgBox.setWindowTitle( rTextShort );
	msgBox.setIcon( QMessageBox::Question );
	msgBox.setInformativeText( rTextLong );
	msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
	int buttonPressed = msgBox.exec();
	rAnswer = ( buttonPressed == QMessageBox::Yes );
	rCancel = ( buttonPressed == QMessageBox::Cancel );
}

inline void SHOW_DIALOG_COMBO_BOX( const QString& rTextShort, const QString& rTextLong, const QStringList& rList, QString& rAnswer, bool& rCancel ) {
	QGMDialogComboBox dlgComboBox;
	dlgComboBox.setTextLabel( rTextLong );
	dlgComboBox.setWindowTitle( QString( rTextShort ) );

	if ( !rList.isEmpty() ) {
		for ( int i = 0; i < rList.size(); i++ ) {
			dlgComboBox.addItem( QString( rList[i] ), QVariant( rList[i] ) );
		}
	}

	if( dlgComboBox.exec() == QDialog::Rejected ) {
		rCancel = true;
	}
	else {
		rCancel = false;
	}

	rAnswer = QString( dlgComboBox.getSelectedItem().toString() );
}

inline void SHOW_INPUT_DIALOG(const QString& rTextShort, const QString& rTextLong, const QString& rDummy, QString& rAnswer, bool& rCancel ) {
	bool ok;
	QInputDialog dlg;
	dlg.setInputMode( QInputDialog::TextInput );
	dlg.setTextValue( rDummy );
	dlg.setLabelText( rTextLong );
	dlg.setWindowTitle( rTextShort );
	dlg.resize(500,100);
	ok = dlg.exec();

	if( ok ) {
		rAnswer = dlg.textValue();
		rCancel = false;
	}
	else {
		rCancel = true;
	}
}

#endif
