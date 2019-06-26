#ifndef QGMDIALOGTRANSPARENCYSETTINGS_H
#define QGMDIALOGTRANSPARENCYSETTINGS_H

#include <QDialog>
#include <QRadioButton>
//#include <meshQt.h>
#include "meshGL.h"

namespace Ui {
class QGMDialogTransparencySettings;
}

class QGMDialogTransparencySettings : public QDialog
{
    Q_OBJECT

public:
	explicit QGMDialogTransparencySettings(QWidget *parent = nullptr);
    ~QGMDialogTransparencySettings();

    void init(MeshGL* mesh, bool enableGL4Features);


private:

    void resetValues();
    void emitValues();
    void showALoopSettings(bool val);

    Ui::QGMDialogTransparencySettings *ui;
    int m_TransparencyMethod;
    int m_BufferMethod;

    double m_transVal;
    double m_transVal2;
    double m_Gamma;

    int m_selLabel;
    int m_numLayers;
    int m_overflowHandling;

    int m_checkedBox;
    int m_checkedBufferMethod;

    MeshGL* m_MGL;

    bool m_enableGL4_3Features;


private slots:
    void exitAccept();
    void exitReject();

    void radioButtonChanged(QAbstractButton* button,bool checked);
    void intValueChanged();
    void transValChanged(int value);
    void transValChanged(double value);

    void transValChanged2(int value);
    void transValChanged2(double value);

    void gammaChanged(int value);
    void gammaChanged(double value);

    void comboBoxChanged();

signals:
    void valuesChanged();

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;

};

#endif // QGMDIALOGTRANSPARENCYSETTINGS_H
