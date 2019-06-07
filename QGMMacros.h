#ifndef QGMMACROS_H
#define QGMMACROS_H

#include "QGMDialogComboBox.h"

//! Helpfull stuff to prevent copy & paste for changing the GUI. 

#define QSETCOLOR( someqobject, r, g, b ) { \
	QPalette p = someqobject->palette(); \
	p.setColor( QPalette::Base, QColor( r, g, b ) ); \
	someqobject->setPalette( p ); \
}

#define _GIGAMESH_LOGO_ ":/GMGeneric/GigaMesh_Logo.png"

#define SHOW_MSGBOX_INFO( rTextShort, rTextLong ) {       \
	QMessageBox msgBox;                               \
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) ); \
	msgBox.setWindowTitle( rTextShort );              \
	msgBox.setText( rTextShort );                     \
	msgBox.setIcon( QMessageBox::Information );       \
	msgBox.setInformativeText( rTextLong );           \
	msgBox.setStandardButtons( QMessageBox::Ok );     \
	msgBox.exec();                                    \
};

#define SHOW_MSGBOX_INFO_SAVE( rTextShort, rTextLong, rTargetObject, rTargetSlot ) {            \
	QMessageBox msgBox;                                                                     \
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );                                       \
	msgBox.setWindowTitle( rTextShort );                                                    \
	msgBox.setText( rTextShort );                                                           \
	msgBox.setIcon( QMessageBox::Information );                                             \
	msgBox.setInformativeText( rTextLong );                                                 \
	msgBox.setStandardButtons( QMessageBox::Ok );                                           \
	QPushButton* saveButtonPtr = msgBox.addButton( QMessageBox::Save );                     \
	QObject::connect( saveButtonPtr, &QPushButton::clicked, rTargetObject, rTargetSlot );   \
	msgBox.exec();                                                                          \
};

#define SHOW_MSGBOX_WARN( rTextShort, rTextLong ) {       \
	QMessageBox msgBox;                               \
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) ); \
	msgBox.setWindowTitle( rTextShort );              \
	msgBox.setText( rTextShort );                     \
	msgBox.setIcon( QMessageBox::Warning );           \
	msgBox.setInformativeText( rTextLong );           \
	msgBox.setStandardButtons( QMessageBox::Ok );     \
	msgBox.exec();                                    \
};

#define SHOW_MSGBOX_WARN_TIMEOUT( rTextShort, rTextLong, rTimeOut ) {  \
	QMessageBox *msgBox = new QMessageBox;                         \
	msgBox->setAttribute( Qt::WA_DeleteOnClose, true );            \
	msgBox->setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );             \
	msgBox->setWindowTitle( rTextShort );                          \
	msgBox->setText( rTextShort );                                 \
	msgBox->setIcon( QMessageBox::Warning );                       \
	msgBox->setInformativeText( rTextLong );                       \
	msgBox->setStandardButtons( QMessageBox::Ok );                 \
	msgBox->show();                                                \
	QTimer::singleShot( rTimeOut, msgBox, SLOT(close()) );         \
};

#define SHOW_MSGBOX_CRIT( rTextShort, rTextLong ) {       \
	QMessageBox msgBox;                               \
	msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) ); \
	msgBox.setWindowTitle( rTextShort );              \
	msgBox.setText( rTextShort );                     \
	msgBox.setIcon( QMessageBox::Critical );          \
	msgBox.setInformativeText( rTextLong );           \
	msgBox.setStandardButtons( QMessageBox::Ok );     \
	msgBox.exec();                                    \
};

#define SHOW_QUESTION( rTextShort, rTextLong, rAnswer, rCancel ) {                          \
    QMessageBox msgBox;                                                                     \
    msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );                                       \
    msgBox.setWindowTitle( rTextShort );                                                    \
    msgBox.setIcon( QMessageBox::Question );                                                \
    msgBox.setInformativeText( rTextLong );                                                 \
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );  \
    int buttonPressed = msgBox.exec();                                                      \
    rAnswer = ( buttonPressed == QMessageBox::Yes );                                        \
    rCancel = ( buttonPressed == QMessageBox::Cancel );                                     \
};

#define SHOW_DIALOG_COMBO_BOX( rTextShort, rTextLong, rList, rAnswer, rCancel ) {           \
    QGMDialogComboBox dlgComboBox;                                                          \
    dlgComboBox.setTextLabel( rTextLong );                                      \
    dlgComboBox.setWindowTitle( QString( rTextShort ) );                                                \
                                                                                            \
    if ( !rList.isEmpty() ) {                                                               \
        for ( int i = 0; i < rList.size(); i++ ) {                                          \
            dlgComboBox.addItem( QString( rList[i] ), QVariant( rList[i] ) );               \
        }                                                                                   \
    }                                                                                       \
                                                                                            \
    if( dlgComboBox.exec() == QDialog::Rejected ) {                                         \
        rCancel = true;                                                                     \
    }                                                                                       \
    else {                                                                                  \
        rCancel = false;                                                                    \
    }                                                                                       \
                                                                                            \
    rAnswer = QString( dlgComboBox.getSelectedItem().toString() );                          \
};

#define SHOW_INPUT_DIALOG(rTextShort, rTextLong, rDummy, rAnswer, rCancel ) {               \
    bool ok;                                                                                \
    QInputDialog dlg;                                                                       \
    dlg.setInputMode( QInputDialog::TextInput );                                            \
    dlg.setTextValue( rDummy );                                                             \
    dlg.setLabelText( rTextLong );                                                          \
    dlg.setWindowTitle( rTextShort );                                                       \
    dlg.resize(500,100);                                                                    \
    ok = dlg.exec();                                                                        \
                                                                                            \
    if( ok ) {                                                                              \
        rAnswer = dlg.textValue();                                                          \
        rCancel = false;                                                                    \
    }                                                                                       \
    else {                                                                                  \
        rCancel = true;                                                                     \
    }                                                                                       \
};

#endif
