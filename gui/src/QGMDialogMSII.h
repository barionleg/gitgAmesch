#ifndef QGMDIALOGMSII_H
#define QGMDIALOGMSII_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include <GigaMesh/mesh/gmcommon.h>
#include <filesystem>

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>
#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogMSII.h"

// Qt includes:
#include "QGMMacros.h"


struct ParamsMSII {
	// Required information:
	int     seedVertexId;
	bool    IdIsOri;
	double  radius;
	int     multiscaleRadiiNr;      //!< Number of radii in multiscaleRadii.
	double* multiscaleRadii;        //!< Relative radii - have to be between 0.0 and 1.0
	int     cubeEdgeLengthInVoxels;
	// Debuging and testing:
	bool    dumpAsMatlabString;
	bool    writeToMesh;
	std::filesystem::path  fileNameMesh;
	bool    writeRaster;
	std::filesystem::path  fileNameRaster;
	bool    writeFilterMasks;
	std::filesystem::path  fileNameFilterMasks;
	bool    writeFilterResult;
	std::filesystem::path  fileNameFilterResult;
};

//!
//! \brief Dialog class for GigaMesh's fetchSphereVolume function (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogMSII : public QDialog, private Ui::dialogMSII {
    Q_OBJECT

public:
	// Constructor and Destructor:
	QGMDialogMSII( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

	// Set values:
	void setVertex( int vertexID );

	// Retrieve Dialog Values:
	bool    isOriIndex();
	int     getIndex();
	double  getRadius();
	bool    getMultiScaleRadii( double** multiscaleRadii, int* multiscaleRadiiNr );
	int     getCubeEdgeLengthInVoxels();
	bool    writeToMesh();
	QString getFileNameMesh();
	bool    writeRaster();
	QString getFileNameRaster();
	bool    writeFilterMasks();
	QString getFileNameFilterMasks();
	bool    writeFilterResult();
	QString getFileNameFilterResult();
	bool    getDumpAsMatlabString();

public slots:
	void accept() final;

signals:
	void estimateMSIIFeat(ParamsMSII); //!< emitted when this dialog is successfully closed - see: MeshQt::slotFetchSphereVolume2 and accept()

private:
	int    primitiveIdx;           //!< selected index (already converted from QLineEdit to integer).
	double radius;                 //!< selected radius (already converted  from QLineEdit to float).
	int    cubeEdgeLengthInVoxels; //!< selected voxel-cube size (already converted from QLineEdit to integer).

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif
