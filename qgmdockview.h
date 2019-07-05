#ifndef QGMDOCKVIEW_H
#define QGMDOCKVIEW_H

#include <QDockWidget>

#include "meshwidget_params.h"
#include "meshGL_params.h"

namespace Ui {
class QGMDockView;
}

class QGMDockView : public QDockWidget
{
	Q_OBJECT

public:
	explicit QGMDockView(QWidget *parent = 0);
	~QGMDockView();

public slots:
	void viewPortInfo(const MeshWidgetParams::eViewPortInfo rInfoID, const QString& rInfoString );
	void infoMesh(const MeshGLParams::eInfoMesh rInfoID, const QString& rInfoString );

private:
	Ui::QGMDockView *ui;

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKVIEW_H
