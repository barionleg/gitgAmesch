#ifndef QGMDialogEnterText_H
#define QGMDialogEnterText_H

// generic C++ includes:
#include <iostream>
#include <set>

// C++ includes:
#include "gmcommon.h"

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>
//#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogEnterText.h"

// Qt includes:
#include "QGMMacros.h"
//!
//! \brief Dialog class for simple user input.
//!
//! Emits signal with text entered as string, int and float.
//! Disconnects itself so it can be used for other purposes.
//!
//! Layer 2
//!

class QGMDialogEnterText : public QDialog, private Ui::QGMDialogEnterText {
    Q_OBJECT

public:
	// Constructor and Destructor:
	QGMDialogEnterText( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );
	// Setup -----------------------------------------------------------------------------------------------------------------------------------------------
	void setID( int rID );
	void setText( const std::string& rText );
	void setText( const QString& rText );
	void setText( const std::set<long>& rValues );
	void setText( std::vector<double>& rValues );
	void fetchClipboard();
	void setDouble( double rValue );
	void setInt( int rValue );
	void setuInt( unsigned int rValue );

	// Retrieve entered information ------------------------------------------------------------------------------------------------------------------------
	bool getText( std::string& rString );
	bool getText( QString* rString );
	bool getText( int* rValue );
	bool getText( unsigned int* rValue );
	bool getText( uint64_t* rValue );
	bool getText( double* rValue );
	bool getText( std::set<long>&      rSet    );
	bool getText( std::vector<long>&   rVector );
	bool getText( std::vector<double>& rVector );

public slots:
	void accept();

signals:
	// Simple signals - new Qt5 style: no overloading allowed/possible.
	void textEnteredStr(QString);     //!< emitted when this dialog is successfully closed - see: accept()

	// Simple signals:
	void textEntered(int);         //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(unsigned int);         //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(float);       //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(double);      //!< emitted when this dialog is successfully closed - see: accept()
	// Signals with ID:
	void textEntered(int,QString); //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,int);     //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,unsigned int);     //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,float);   //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,double);  //!< emitted when this dialog is successfully closed - see: accept()
	// Array/vector signals
	void textEntered(std::vector<int>);    //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(std::vector<double>); //!< emitted when this dialog is successfully closed - see: accept()

private:
	int mValueID; //!< Optional ID to identify values to be set, e.g. by MeshGL::setViewParams
};

#endif
