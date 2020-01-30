#ifndef QGMDOCKINFO_H
#define QGMDOCKINFO_H

#include <QDockWidget>
#include <QVariant>

#include <GigaMesh/mesh/mesh_params.h>
#include "meshwidget_params.h"

namespace Ui {
class QGMDockInfo;
}

class QGMDockInfo : public QDockWidget
{
	Q_OBJECT
	
public:
	explicit QGMDockInfo( QWidget *parent=0 );
	~QGMDockInfo();

public slots:
	// ---- Mouse Mode ------------------------------------------------------------------------
	void selectMouseModeDefault();
	void selectMouseModeExtra( bool rActive, MeshWidgetParams::eMouseModes rMode );
	// ---- User Guide ------------------------------------------------------------------------
	void setGuideIDCommon(    MeshWidgetParams::eGuideIDCommon    rGuideID );
	void setGuideIDSelection( MeshWidgetParams::eGuideIDSelection rGuideID );
	// --- Progress bar -----------------------------------------------------------------------
	void showProgressStart(const QString& rMsg );
	void showProgress( double rVal );
	void showProgressStop(const QString& rMsg );

private slots:
	// --- Private slots ----------------------------------------------------------------------
	void setMouseMode(const int rComboBoxIdx );
	void showProgressStopReset();
	void visibility( bool rVisible );

signals:
	void sReloadFromFile();                                         //!< Request the file to be reloaded from file.
	void sShowFlagMeshWidget(MeshWidgetParams::eParamFlag,bool);    //!< Sets a specific display flag see MeshWidgetParams::setShowFlag
	void sShowParamIntMeshWidget(MeshWidgetParams::eParamInt,int);  //!< Sets a specific param int see MeshWidgetParams::setViewParamsInt

private:
	Ui::QGMDockInfo *ui;

	// Mouse mode related:
	MeshWidgetParams::eMouseModes mMouseMode;        //! Storage for the current Mouse Mode.
	QVariant mMouseModeNoControlKey = MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA;                 //! Temporary storage for the mouse mode, when the Control Key is pressed.
	// Guide related:
	MeshWidgetParams::eGuideIDCommon    mGuideCommon;      //! Storage for the last/current Selection Guide ID.
	MeshWidgetParams::eGuideIDSelection mGuideSelection;   //! Storage for the last/current Selection Guide ID in selection mode.

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKINFO_H
