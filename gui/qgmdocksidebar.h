#ifndef QGMDOCKSIDEBAR_H
#define QGMDOCKSIDEBAR_H

#include <QDockWidget>

#include "meshGL_params.h"

namespace Ui {
class QGMDockSideBar;
}

class QGMDockSideBar : public QDockWidget {
	Q_OBJECT
	
public:
	explicit QGMDockSideBar(QWidget *parent = 0);
	~QGMDockSideBar();

private slots:
	void texMapVertMono( bool rState );
	void texMapVertRGB( bool rState );
	void texMapVertFuncVal( bool rState );
	void texMapVertLabels( bool rState );

	void shaderChoiceWireframe( bool rState );
	void shaderChoiceNPR( bool rState );
	void shaderChoicePointCloud( bool rState);
	void shaderChoiceMonolithic( bool rState);
	void shaderChoiceTransparency( bool rState);
	void shaderChoiceTextured( bool rState);
public slots:
	void updateMeshParamInt( MeshGLParams::eParamInt rParamNr, int rSetValue );
	void enableTextureMeshRendering(bool enable);

signals:
	void sShowParamIntMeshGL(MeshGLParams::eParamInt,int); //!< Trigger setup of a MeshGL specific parameter of type int.
	void sShowNPRSettings();
	void sShowTransparencySettings();

private:
	Ui::QGMDockSideBar *ui;

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKSIDEBAR_H
