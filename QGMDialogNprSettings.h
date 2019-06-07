#ifndef QGMDIALOGNPRSETTINGS_H
#define QGMDIALOGNPRSETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QRadioButton>
#include "gmcommon.h"
#include "meshGL.h"
#include "meshwidget.h"

namespace Ui {
class QGMDialogNprSettings;
}

class QGMDialogNprSettings : public QDialog
{
    Q_OBJECT

public:
    explicit QGMDialogNprSettings(QWidget *parent = 0);
    ~QGMDialogNprSettings();

	void init( MeshGL* rMesh, MeshGLColors* rRenderColors );

private:

    void emitValues();
    void resetValues();

    Ui::QGMDialogNprSettings *ui;
    double m_outlineSize;
    double m_outlineThreshold;
    double m_hatchSize;
    double m_hatchRotation;
    double m_specularSize;

    bool m_bOutlineEnabled;
    bool m_bHatchEnabled;
    bool m_bToonEnabled;
    bool m_bPreview;
    bool m_bSpecularEnabled;

    bool m_bColorChanged;

    bool m_bHatchLInfluence;
    int m_hatchStyle;

    int m_hatchSource;
    int m_toonSource;
    int m_outlineSource;

    int m_toonType;
    int m_toonLightingSteps;
    int m_toonHueSteps;
    int m_toonSatSteps;
    int m_toonValSteps;

    QColor m_OutlineColor;
    QColor m_HatchColor;
    QColor m_Diffuse1;
    QColor m_Diffuse2;
    QColor m_Diffuse3;
    QColor m_Diffuse4;
    QColor m_Diffuse5;
    QColor m_Specular;

    QColorDialog* m_pColorDialog;
    QObject* m_pColorRequest;

	MeshGL*         m_meshGL;
	MeshGLColors*   mRenderColors;

private slots:
    void outlineSizeChanged(int value);
    void outlineSizeChanged(double value);

    void outlineThresholdChanged(int value);
    void outlineThresholdChanged(double value);

    void hatchSizeChanged(int value);
    void hatchSizeChanged(double value);

    void hatchRotationChanged(int value);
    void hatchRotationChanged(double value);

    void specularSizeChanged(int value);
    void specularSizeChanged(double value);

    void comboBoxChanged();

    void changeColor();
    void colorChanged(const QColor& color);

    void changeBool();

    void changeToonType(QAbstractButton* button,bool checked);
    void changeIntSpinner(int value);

    void exitAccept();
    void exitReject();

signals:
    void valuesChanged();

};

#endif // QGMDIALOGNPRSETTINGS_H
