#ifndef QGMDialogPlaneParam_H
#define QGMDialogPlaneParam_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include "gmcommon.h"
#include "vector3d.h"

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>

// OpenGL stuff
#define GL_GLEXT_PROTOTYPES
#if QT_VERSION >= 0x050000
#include <QMessageBox>
#endif

// Qt Interface includes:
#include "ui_dialogPlaneParam.h"

// Qt includes:
#include "QGMMacros.h"

//!
//! \brief Dialog class for GigaMesh's Plane Parameters (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogPlaneParam : public QDialog, private Ui::dialogPlaneParam {
      Q_OBJECT

public:
	// Constructor and Destructor:
	QGMDialogPlaneParam( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

public slots:
	bool setPlaneHNF( Vector3D rPlaneHNF );
	void preferSet();
	void preferVisDist();
	void preferSplit();
	void accept();

signals:
	void sComputeDistance(Vector3D,bool);
	void sSetPlaneHNF(Vector3D);
	void sSplitByPlane(Vector3D,bool);

};

#endif
