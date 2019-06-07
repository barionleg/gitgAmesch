#include "QGMDialogMSII.h"

QGMDialogMSII::QGMDialogMSII( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ),
      primitiveIdx( -1 ),
      radius( _NOT_A_NUMBER_ ),
      cubeEdgeLengthInVoxels( -1 ) {
	//! Constructor
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose, false );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}

void QGMDialogMSII::setVertex( int vertexID ) {
	//! Set the form entry for the Vertex ID
	lineVertex->setText( tr( "%1" ).arg( vertexID ) );
}

bool QGMDialogMSII::isOriIndex() {
	//! User selection: original Index (as loaded from file) VS current Index
	return checkBoxOridIdx->isChecked();
}

int QGMDialogMSII::getIndex() {
	//! Retrieve Primitive index entered, checked and converted.
	//! Negative when no proper value was entered.
	return primitiveIdx;
}

double QGMDialogMSII::getRadius() {
	//! Retrieve radius entered, checked and converted.
	//! Not-A-Number when no proper value was entered.
	return radius;
}

bool QGMDialogMSII::getMultiScaleRadii( double** multiscaleRadii, int* multiscaleRadiiNr ) {
	//! Returns an array with relative radii for multi-scale analysis - values entered have to be within 0.0 and 1.0.
	//! Returns false in case of an error.

	// Check for an empty string:
	if( lineEditRadiiRel->text().size() <= 0 ) {
		QSETCOLOR( lineEditRadiiRel, 255, 128, 128 ); // light read
		return false;
	}

	// Check if all values, sperarated by spaces convert to double and are between 0.0 and 1.0
	QStringList strListRadii = lineEditRadiiRel->text().split( " ", QString::SkipEmptyParts );
	int  radiiCount = strListRadii.size();
	bool nonDoubleRadii = false;
	for( int i=0; i<radiiCount; i++ ) {
		bool convertOk;
		double tmpRadii = strListRadii.at( i ).toDouble( &convertOk );
		if( !convertOk ) {
			nonDoubleRadii = true;
		}
		if( ( tmpRadii < 0.0 ) || ( tmpRadii > 1.0 ) ) {
			nonDoubleRadii = true;
		}
	}
	if( nonDoubleRadii ) {
		QSETCOLOR( lineEditRadiiRel, 255, 128, 128 ); // light read
		return false;
	}

	// Now we can allocate memory and convert:
	(*multiscaleRadiiNr) = radiiCount;
	(*multiscaleRadii)   = new double[radiiCount];
	for( int i=0; i<radiiCount; i++ ) {
		(*multiscaleRadii)[i] = strListRadii.at( i ).toDouble();
	}

	QSETCOLOR( lineRadius, 255, 255, 255 ); // white
	return true;
}

int QGMDialogMSII::getCubeEdgeLengthInVoxels() {
	//! Retrieve cube size in voxels entered, checked and converted.
	//! Negative when no proper value was entered.
	return cubeEdgeLengthInVoxels;
}

bool QGMDialogMSII::writeToMesh() {
	//! User selection about writing the fetched Mesh to a file (using MeshIO::writeFile).
	return checkBoxWriteMesh->isChecked();
}

QString QGMDialogMSII::getFileNameMesh() {
	//! Filename the fetched Mesh should be written to.
	return lineFileNameMesh->text();
}

bool QGMDialogMSII::writeRaster() {
	//! User selection about writing the fetched Mesh to a file (using MeshIO::writeFile).
	return checkBoxWriteRaster->isChecked();
}

QString QGMDialogMSII::getFileNameRaster() {
	//! Filename the fetched Mesh should be written to.
	return lineFileNameRaster->text();
}

bool QGMDialogMSII::writeFilterMasks() {
	//! User selection about writing the filter masks to a file (using MeshIO::writeFile).
	return checkBoxFilterMasks->isChecked();
}

QString QGMDialogMSII::getFileNameFilterMasks() {
	//! Filename the fetched Mesh should be written to.
	return lineFileNameFilterMasks->text();
}

bool QGMDialogMSII::writeFilterResult() {
	//! User selection about writing the fetched Mesh to a file (using MeshIO::writeFile).
	return checkBoxWriteFilter->isChecked();
}

QString QGMDialogMSII::getFileNameFilterResult() {
	//! Filename the fetched Mesh should be written to.
	return lineFileNameFilter->text();
}

bool QGMDialogMSII::getDumpAsMatlabString() {
	//! User selection about dumping information to the console in Matlab syntax.
	return checkBoxDumpMatlab->isChecked();
}

// --- SLOTS ---------------------------------------------------------------------------

void QGMDialogMSII::accept() {
	//! Checks if all the input values are correct. If the values seem to be correct
	//! a signal is emitted, which should be connected MeshQt.
	//cout << "[QGMMainWindow::accept] lineVertex: " << lineRadius->text().toStdString() << endl;

	ParamsMSII newParamsMSII;
	bool       valuesBad = false;
	bool       convertOk;

	primitiveIdx = lineVertex->text().toInt( &convertOk );
	if( ( !convertOk ) || ( primitiveIdx < 0 ) ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineVertex, 255, 128, 128 ); // light read
		primitiveIdx = -1;
		valuesBad = true;
	} else {
		QSETCOLOR( lineVertex, 255, 255, 255 ); // white
	}

	radius = lineRadius->text().toDouble( &convertOk );
	if( ( !convertOk ) || ( radius <= 0.0 ) ) {
		QSETCOLOR( lineRadius, 255, 128, 128 ); // light read
		radius = _NOT_A_NUMBER_DBL_;
		valuesBad = true;
	} else {
		QSETCOLOR( lineRadius, 255, 255, 255 ); // white
	}
	//cout << "[QGMMainWindow::accept] lineRadius: " << radius << endl;

	if( !getMultiScaleRadii( &newParamsMSII.multiscaleRadii, &newParamsMSII.multiscaleRadiiNr ) ) {
		valuesBad = true;
	}

	cubeEdgeLengthInVoxels = lineCubeSize->text().toInt( &convertOk );
	if( ( !convertOk ) || ( cubeEdgeLengthInVoxels <= 0 ) ) {
		QSETCOLOR( lineCubeSize, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineCubeSize, 255, 255, 255 ); // white
	}

	if( valuesBad ) {
		return;
	}

	newParamsMSII.seedVertexId           = getIndex();
	newParamsMSII.IdIsOri                = isOriIndex();
	newParamsMSII.radius                 = getRadius();
	newParamsMSII.cubeEdgeLengthInVoxels = getCubeEdgeLengthInVoxels();
	newParamsMSII.dumpAsMatlabString     = getDumpAsMatlabString();
	newParamsMSII.writeToMesh            = writeToMesh();
	newParamsMSII.fileNameMesh           = getFileNameMesh().toStdString();
	newParamsMSII.writeRaster            = writeRaster();
	newParamsMSII.fileNameRaster         = getFileNameRaster().toStdString();
	newParamsMSII.writeFilterMasks       = writeFilterMasks();
	newParamsMSII.fileNameFilterMasks    = getFileNameFilterMasks().toStdString();
	newParamsMSII.writeFilterResult      = writeFilterResult();
	newParamsMSII.fileNameFilterResult   = getFileNameFilterResult().toStdString();
	//cout << "[QGMMainWindow::accept] Emit Signal." << endl;
	emit estimateMSIIFeat( newParamsMSII );

	// to prevent multiple and wrong connections:
	QObject::disconnect();

	QDialog::accept();
}
