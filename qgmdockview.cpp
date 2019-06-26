#include "qgmdockview.h"
#include "ui_qgmdockview.h"

//! Constructor.
QGMDockView::QGMDockView( QWidget *parent ) :
        QDockWidget( parent ),
        ui( new Ui::QGMDockView ) {
	ui->setupUi(this);
}

//! Destructor.
QGMDockView::~QGMDockView() {
	delete ui;
}

//! Handles incoming informations of the MeshWidget.
void QGMDockView::viewPortInfo( const MeshWidgetParams::eViewPortInfo rInfoID, const QString& rInfoString ) {
	switch( rInfoID ) {
		case MeshWidgetParams::VPINFO_FRAMES_PER_SEC:
			ui->labelFPSValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_DPI:
			ui->labelDPIValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_LIGHT_CAM:
			ui->labelLightCamValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_LIGHT_WORLD:
			ui->labelLightWorldValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_FUNCTION_VALUE:
			ui->labelFuncValValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_LABEL_ID:
			ui->labelLabelValue->setText( rInfoString );
			break;
	}
}

//! Handles incoming informations of the MeshQt.
void QGMDockView::infoMesh( const MeshGLParams::eInfoMesh rInfoID, const QString& rInfoString ) {
	switch( rInfoID ) {
		case MeshGLParams::MGLINFO_SELECTED_PRIMITIVE:
			ui->labelSelPrimValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_VERTICES:
			ui->labelSelMVertsValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_FACES:
			ui->labelSelMFacesValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_POLYLINES:
			ui->labelSelMPolysValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_POSITIONS:
			ui->labelPositionsValue->setText( rInfoString );
			break;
	}
}


void QGMDockView::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDockWidget::changeEvent(event);
}
