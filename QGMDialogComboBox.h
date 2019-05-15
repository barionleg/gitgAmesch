#ifndef QGMDIALOGCOMBOBOX_H
#define QGMDIALOGCOMBOBOX_H

// generic C++ includes:
//#include <iostream>

// C++ includes:
#include "gmcommon.h"

// generic Qt includes:
#include <QAction>
#include <QDialog>
#include <qstring.h>
//#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogComboBox.h"

// Qt includes:
#include "QGMMacros.h"

//!
//! \brief Dialog class for simple user input having a CommboBox and a TextLabel.
//!
//! Emits signal with the ID and/or of the select item.
//! Disconnects itself so it can be used for other purposes.
//!
//! Layer 2
//!

class QGMDialogComboBox : public QDialog, private Ui::QGMDialogComboBox {
    Q_OBJECT

public:
	explicit QGMDialogComboBox( QWidget *parent = 0, Qt::WindowFlags flags = 0  );
	// Setup -----------------------------------------------------------------------------------------------------------------------------------------------
	void setTextLabel( const QString & rLabelText );
	void addItem( const QString & text, const QVariant & userData = QVariant() );

	// Retrieve selected information -----------------------------------------------------------------------------------------------------------------------
	QVariant getSelectedItem();

signals:

public slots:

};

#endif // QGMDIALOGCOMBOBOX_H
