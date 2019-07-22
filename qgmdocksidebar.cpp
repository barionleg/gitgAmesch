#include "qgmdocksidebar.h"
#include "ui_qgmdocksidebar.h"

#include "QGMMainWindow.h"
#include <QStyle>

using namespace std;

QGMDockSideBar::QGMDockSideBar( QWidget *parent ) :
	QDockWidget( parent ), ui( new Ui::QGMDockSideBar )
{
	ui->setupUi( this );

	QObject::connect( ui->radioButtonSolidColor,       SIGNAL(toggled(bool)), this, SLOT(texMapVertMono(bool))       );
	QObject::connect( ui->radioButtonVertexTex,        SIGNAL(toggled(bool)), this, SLOT(texMapVertRGB(bool))        );
	QObject::connect( ui->radioButtonFuncValVertTex,   SIGNAL(toggled(bool)), this, SLOT(texMapVertFuncVal(bool))    );
	QObject::connect( ui->radioButtonVertexLabel,      SIGNAL(toggled(bool)), this, SLOT(texMapVertLabels(bool))     );
	QObject::connect( ui->radioButtonWireframe,        SIGNAL(toggled(bool)), this, SLOT(shaderChoiceWireframe(bool))        );
	QObject::connect( ui->radioButtonNPRendering,      SIGNAL(toggled(bool)), this, SLOT(shaderChoiceNPR(bool))    );
	QObject::connect( ui->radioButtonPointcloud,       SIGNAL(toggled(bool)), this, SLOT(shaderChoicePointCloud(bool)));
	QObject::connect( ui->radioButton_Solid,           SIGNAL(toggled(bool)), this, SLOT(shaderChoiceMonolithic(bool)));
	QObject::connect( ui->radioButtonTransparency,     SIGNAL(toggled(bool)), this, SLOT(shaderChoiceTransparency(bool)));

	QObject::connect( this, SIGNAL(sShowParamIntMeshGL(MeshGLParams::eParamInt,int)), SLOT(updateMeshParamInt(MeshGLParams::eParamInt,int)) );

	QObject::connect( ui->pushButton_NPR, SIGNAL(pressed()), this, SIGNAL(sShowNPRSettings()));
	QObject::connect( ui->pushButton_transparent, SIGNAL(pressed()), this, SIGNAL(sShowTransparencySettings()));

	ui->pushButton_transparent->setIcon(ui->pushButton_transparent->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	ui->pushButton_NPR->setIcon(ui->pushButton_NPR->style()->standardIcon(QStyle::SP_FileDialogDetailedView));

}

//! Destructor
QGMDockSideBar::~QGMDockSideBar() {
	delete( ui );
}

//! Switch to MeshGL::TEXMAP_VERT_MONO
void QGMDockSideBar::texMapVertMono( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_MONO  );
	}
}

//! Switch to MeshGL::TEXMAP_VERT_RGB
void QGMDockSideBar::texMapVertRGB( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB   );
	}
}

//! Switch to MeshGL::TEXMAP_VERT_FUNCVAL
void QGMDockSideBar::texMapVertFuncVal( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_FUNCVAL );
	}
}

//! Switch to MeshGL::TEXMAP_VERT_LABELS
void QGMDockSideBar::texMapVertLabels( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_LABELS );
	}
}

//! Switch to Wireframe rendering
void QGMDockSideBar::shaderChoiceWireframe( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE,       MeshGLParams::SHADER_WIREFRAME );
		ui->radioButtonFuncValVertTex->setEnabled(false);
		ui->radioButtonVertexLabel->setEnabled(false);
		ui->radioButtonVertexTex->setEnabled(false);
		ui->radioButtonSolidColor->setEnabled(false);
	}
	else
	{
		ui->radioButtonFuncValVertTex->setEnabled(true);
		ui->radioButtonVertexLabel->setEnabled(true);
		ui->radioButtonVertexTex->setEnabled(true);
		ui->radioButtonSolidColor->setEnabled(true);
	}
}

//! Switch to NPR-Rendering
void QGMDockSideBar::shaderChoiceNPR( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_NPR );
		ui->radioButtonFuncValVertTex->setEnabled(false);
		ui->radioButtonVertexLabel->setEnabled(false);
		ui->radioButtonVertexTex->setEnabled(false);
		ui->radioButtonSolidColor->setEnabled(false);
	}
	else
	{
		ui->radioButtonFuncValVertTex->setEnabled(true);
		ui->radioButtonVertexLabel->setEnabled(true);
		ui->radioButtonVertexTex->setEnabled(true);
		ui->radioButtonSolidColor->setEnabled(true);
	}
}

//! Switch to pointcloud rendering
void QGMDockSideBar::shaderChoicePointCloud(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_POINTCLOUD);
	}
}

//! Swtich to monolithic rendering
void QGMDockSideBar::shaderChoiceMonolithic(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_MONOLITHIC);
	}
}

void QGMDockSideBar::shaderChoiceTransparency(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_TRANSPARENCY);
	}
}

//! Sets menu items according to the flags of MeshGL::viewParamsIntArr
void QGMDockSideBar::updateMeshParamInt( MeshGLParams::eParamInt rParamNr, int rSetValue ) {
	ui->radioButtonFuncValVertTex->setEnabled(true);
	ui->radioButtonVertexLabel->setEnabled(true);
	ui->radioButtonVertexTex->setEnabled(true);
	ui->radioButtonSolidColor->setEnabled(true);
	switch( rParamNr ) {
		case MeshGLParams::TEXMAP_CHOICE_FACES:
			switch( static_cast<MeshGLParams::eTexMapType>(rSetValue) ) {
				case MeshGLParams::TEXMAP_VERT_MONO:
					ui->radioButtonSolidColor->setChecked( true );
					break;
				case MeshGLParams::TEXMAP_VERT_RGB:
					ui->radioButtonVertexTex->setChecked( true );
					break;
				case MeshGLParams::TEXMAP_VERT_FUNCVAL:
					ui->radioButtonFuncValVertTex->setChecked( true );
					break;
				case MeshGLParams::TEXMAP_VERT_LABELS:
					ui->radioButtonVertexLabel->setChecked( true );
					break;
				default:
					cerr << "[QGMDockSideBar::" << __FUNCTION__ << "] ERROR: parameter TEXMAP_CHOICE has an unknown choice: " << rSetValue << "!" << endl;
			}
			break;
		case MeshGLParams::SHADER_CHOICE:
			switch( static_cast<MeshGLParams::eShaderChoice>(rSetValue) ) {
				case MeshGLParams::SHADER_MONOLITHIC:
					ui->radioButton_Solid->setChecked( true );
					break;
				case MeshGLParams::SHADER_WIREFRAME:
					ui->radioButtonWireframe->setChecked( true );
					ui->radioButtonFuncValVertTex->setEnabled(false);
					ui->radioButtonVertexLabel->setEnabled(false);
					ui->radioButtonVertexTex->setEnabled(false);
					ui->radioButtonSolidColor->setEnabled(false);
					break;
				case MeshGLParams::SHADER_NPR:
					ui->radioButtonNPRendering->setChecked( true );
					ui->radioButtonFuncValVertTex->setEnabled(false);
					ui->radioButtonVertexLabel->setEnabled(false);
					ui->radioButtonVertexTex->setEnabled(false);
					ui->radioButtonSolidColor->setEnabled(false);
					break;
			case MeshGLParams::SHADER_POINTCLOUD:
					ui->radioButtonPointcloud->setChecked ( true );
					break;
			case MeshGLParams::SHADER_TRANSPARENCY:
					ui->radioButtonTransparency->setChecked( true );
					break;
				default:
					cerr << "[QGMDockSideBar::" << __FUNCTION__ << "] ERROR: parameter SHADER_CHOICE has an unknown choice: " << rSetValue << "!" << endl;
			}
			break;
		case MeshGLParams::VIEWPARAMS_INT_UNDEFINED:
		case MeshGLParams::COLMAP_LABEL_OFFSET:
		case MeshGLParams::PARAMS_INT_COUNT:
		//NPR-params
		case MeshGLParams::NPR_HATCH_SOURCE:
		case MeshGLParams::NPR_HATCH_STYLE:
		case MeshGLParams::NPR_OUTLINE_SOURCE:
		case MeshGLParams::NPR_TOON_SOURCE:
		case MeshGLParams::NPR_TOON_TYPE:
		case MeshGLParams::NPR_TOON_LIGHTING_STEPS:
		case MeshGLParams::NPR_TOON_HUE_STEPS:
		case MeshGLParams::NPR_TOON_SAT_STEPS:
		case MeshGLParams::NPR_TOON_VAL_STEPS:
		case MeshGLParams::DEFAULT_FRAMEBUFFER_ID:
		//transparency params
		case MeshGLParams::TRANSPARENCY_NUM_LAYERS:
		case MeshGLParams::TRANSPARENCY_TRANS_FUNCTION:
		case MeshGLParams::TRANSPARENCY_SEL_LABEL:
		case MeshGLParams::TRANSPARENCY_OVERFLOW_HANDLING:
		case MeshGLParams::TRANSPARENCY_BUFFER_METHOD:

			// nothing to do.
			break;
		default:
			cerr << "[QGMDockSideBar::" << __FUNCTION__ << "] ERROR: unsupported/unimplemented parameter no: " << rParamNr << "!" << endl;
	}
}


void QGMDockSideBar::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDockWidget::changeEvent(event);
}
