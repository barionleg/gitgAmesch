#ifndef QGMDIALOGCOMBOBOX_H
#define QGMDIALOGCOMBOBOX_H

// generic C++ includes:
//#include <iostream>

// C++ includes:
#include <GigaMesh/mesh/gmcommon.h>

// generic Qt includes:
#include <QAction>
#include <QDialog>
#include <qstring.h>
//#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogComboBox.h"

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
	explicit QGMDialogComboBox( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr  );
	// Setup -----------------------------------------------------------------------------------------------------------------------------------------------
	void setTextLabel( const QString & rLabelText );
	void addItem( const QString & text, const QVariant & userData = QVariant() );

	// Retrieve selected information -----------------------------------------------------------------------------------------------------------------------
	QVariant getSelectedItem();

signals:

public slots:


	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGCOMBOBOX_H
