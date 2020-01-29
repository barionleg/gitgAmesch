#include "QGMDialogRuler.h"
#include <QFileIconProvider>
#include <QFileDialog>

QGMDialogRuler::QGMDialogRuler( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	//! Constructor
	setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );

    QFileIconProvider iconProvider;

    lineRulerFileNameSelectButton->setIcon(iconProvider.icon(QFileIconProvider::Folder));
    QObject::connect(lineRulerFileNameSelectButton, SIGNAL(pressed()), this, SLOT(setFileDirectory()));
    _fileDirectory = ".";
}

// --- SLOTS ---------------------------------------------------------------------------

void QGMDialogRuler::setFileName( const QString& fileName ) {
	//! Set filename.
	lineRulerFileName->setText( fileName );
}

void QGMDialogRuler::setWidth( const double width ) {
	//! Set width.
	lineRulerWidth->setText( QString( "%1" ).arg( width ) );
}

void QGMDialogRuler::setWidthUnit( const QString& widthUnit ) {
	//! Set widthUnit.
	lineRulerWidthUnit->setText( widthUnit );
}

void QGMDialogRuler::setHeight( const double height ) {
	//! Set height.
	lineRulerHeight->setText( QString( "%1" ).arg( height ) );
}

void QGMDialogRuler::setUnit( const double unit ) {
	//! Set unit.
	lineRulerUnit->setText( QString( "%1" ).arg( unit ) );
}

void QGMDialogRuler::setUnitTicks( const double unitTicks ) {
	//! Set unit tick marks.
	lineRulerUnitTicks->setText( QString( "%1" ).arg( unitTicks ) );
}

void QGMDialogRuler::accept() {
	//! Checks if all the input values are correct. If the values seem to be correct
	//! a signal is emitted, which should be connected MeshQt.

	bool  valuesBad = false;
	bool  convertOk;

	double width;
	double height;
	double unit;
	double unitTicks;

	width = lineRulerWidth->text().toDouble( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineRulerWidth, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineRulerWidth, 255, 255, 255 ); // white
	}
	// no negative values nor zero permitted:
	if( width <= 0.0 ) {
		QSETCOLOR( lineRulerWidth, 255, 128, 128 ); // light read
		valuesBad = true;
	}

	height = lineRulerHeight->text().toDouble( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineRulerHeight, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineRulerHeight, 255, 255, 255 ); // white
	}
	// no negative values nor zero permitted:
	if( height <= 0.0 ) {
		QSETCOLOR( lineRulerHeight, 255, 128, 128 ); // light read
		valuesBad = true;
	}

	unit = lineRulerUnit->text().toDouble( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineRulerUnit, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineRulerUnit, 255, 255, 255 ); // white
	}
	// no negative values nor zero permitted:
	if( unit <= 0.0 ) {
		QSETCOLOR( lineRulerUnit, 255, 128, 128 ); // light read
		valuesBad = true;
	}

	unitTicks = lineRulerUnitTicks->text().toDouble( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineRulerUnitTicks, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineRulerUnitTicks, 255, 255, 255 ); // white
	}
	// no negative values permitted:
	if( unitTicks < 0.0 ) {
		QSETCOLOR( lineRulerUnitTicks, 255, 128, 128 ); // light read
		valuesBad = true;
	}

  if( lineRulerWidthUnit->text().toStdString().compare("mm") &
      lineRulerWidthUnit->text().toStdString().compare("cm") &
      lineRulerWidthUnit->text().toStdString().compare("dm") & 
      lineRulerWidthUnit->text().toStdString().compare("m") ) {
    QSETCOLOR( lineRulerWidthUnit, 255, 128, 128 ); // light read
    valuesBad = true;
  } else {
    QSETCOLOR( lineRulerWidthUnit, 255, 255, 255 ); // white
  }

	// No accept, when one or more values are incorrect.
	if( valuesBad ) {
		return;
	}

	emit setParamFloatMeshWidget( MeshWidgetParams::RULER_WIDTH,      width     );
	emit setParamFloatMeshWidget( MeshWidgetParams::RULER_HEIGHT,     height    );
	emit setParamFloatMeshWidget( MeshWidgetParams::RULER_UNIT,       unit      );
	emit setParamFloatMeshWidget( MeshWidgetParams::RULER_UNIT_TICKS, unitTicks );
	emit setViewParamsStr( MeshWidgetParams::RULER_WIDTH_UNIT, lineRulerUnit->text());
    emit screenshotRuler( _fileDirectory + "/" + lineRulerFileName->text() );
	QDialog::accept();
}

void QGMDialogRuler::setFileDirectory()
{
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly);

	_fileDirectory = fileDialog.getExistingDirectory(this, tr( "Choose the Directory"));
}


void QGMDialogRuler::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
