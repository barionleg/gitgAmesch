#ifndef QGMDIALOGABOUT_H
#define QGMDIALOGABOUT_H

#include <QtGui>
#include <QDialog>

// Qt Interface includes:
#include "ui_dialogAbout.h"

// Qt includes:
#include "QGMMacros.h"

class QGMDialogAbout : public QDialog, private Ui::dialogAbout {
    Q_OBJECT

public:
	QGMDialogAbout( QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::FramelessWindowHint );

signals:

public slots:


	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGABOUT_H
