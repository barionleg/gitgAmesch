#ifndef QGMDIALOGCUTOFFVALUES_H
#define QGMDIALOGCUTOFFVALUES_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include "primitive.h"

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>
#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogCutOffValues.h"

// Qt includes:
#include "QGMMacros.h"

//!
//! \brief Dialog class for GigaMesh's Cut-Off-Feature-Elements function e.g. to remove outliers (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogCutOffValues : public QDialog, private Ui::dialogCutOffValues {
    Q_OBJECT

public:
	// Constructor and Destructor:
	QGMDialogCutOffValues( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

public slots:
	void accept();

signals:
	void cutOffValues(double,double,bool); //!< Cut-Off (e.g. removal of outliers) minVal, maxVal, setToNotANumber

private:

};

#endif
