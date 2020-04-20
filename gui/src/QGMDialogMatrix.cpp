#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "QGMDialogMatrix.h"
#include "ui_QGMDialogMatrix.h"

#include <QClipboard>
#include <QDoubleValidator>
#include <QDoubleSpinBox>
#include <GigaMesh/logging/Logging.h>


QGMDialogMatrix::QGMDialogMatrix(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QGMDialogMatrix)
{
	ui->setupUi(this);

	mLineEditPtrs[ 0] = ui->lineEdit_mat00;
	mLineEditPtrs[ 1] = ui->lineEdit_mat10;
	mLineEditPtrs[ 2] = ui->lineEdit_mat20;
	mLineEditPtrs[ 3] = ui->lineEdit_mat30;

	mLineEditPtrs[ 4] = ui->lineEdit_mat01;
	mLineEditPtrs[ 5] = ui->lineEdit_mat11;
	mLineEditPtrs[ 6] = ui->lineEdit_mat21;
	mLineEditPtrs[ 7] = ui->lineEdit_mat31;

	mLineEditPtrs[ 8] = ui->lineEdit_mat02;
	mLineEditPtrs[ 9] = ui->lineEdit_mat12;
	mLineEditPtrs[10] = ui->lineEdit_mat22;
	mLineEditPtrs[11] = ui->lineEdit_mat32;

	mLineEditPtrs[12] = ui->lineEdit_mat03;
	mLineEditPtrs[13] = ui->lineEdit_mat13;
	mLineEditPtrs[14] = ui->lineEdit_mat23;
	mLineEditPtrs[15] = ui->lineEdit_mat33;

	mValidator.setLocale(QLocale::c());

	uint8_t index = 0;
	for(auto lineEditPtr : mLineEditPtrs)
	{
		lineEditPtr->setValidator(&mValidator);
		lineEditPtr->setProperty("index", index++);
	}

	//tabwidget events
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &QGMDialogMatrix::tabChanged);
	tabChanged(ui->tabWidget->currentIndex());

	//free edit widgets
	connect(ui->pushButton_CopyToClipboard, &QPushButton::clicked, this, &QGMDialogMatrix::copyToClipboard);
	connect(ui->pushButton_loadFromClipboard, &QPushButton::clicked, this, &QGMDialogMatrix::fetchClipboard);

	//translate
	connect(ui->doubleSpinBox_transX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateTranslate);
	connect(ui->doubleSpinBox_transY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateTranslate);
	connect(ui->doubleSpinBox_transZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateTranslate);

	//scale
	connect(ui->doubleSpinBox_scaleX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateScale);
	connect(ui->doubleSpinBox_scaleY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateScale);
	connect(ui->doubleSpinBox_scaleZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateScale);

	//rotate
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisX   , 0);
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisY   , 1);
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisZ   , 2);
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisFree, 3);
	connect(ui->doubleSpinBox_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateRotate);

	auto updateRotValue = [this]() {updateRotate(ui->doubleSpinBox_angle->value());};

	connect(ui->doubleSpinBox_axisX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateRotValue);
	connect(ui->doubleSpinBox_axisY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateRotValue);
	connect(ui->doubleSpinBox_axisZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateRotValue);

	connect(ui->buttonGroup_axisSel, QOverload<int>::of(&QButtonGroup::buttonClicked), updateRotValue);

	connect(ui->horizontalSlider_angle, &QSlider::valueChanged, this, &QGMDialogMatrix::updateRotateBySlider);


	connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &QGMDialogMatrix::resetValues);

	updateMatrixValues();
}

QGMDialogMatrix::~QGMDialogMatrix()
{
	delete ui;
}

void QGMDialogMatrix::fetchClipboard()
{
	QString clipBoardStr = QApplication::clipboard()->text( QClipboard::Clipboard );

	const QStringList values = clipBoardStr.simplified().split( " ", QString::SkipEmptyParts);

	if(values.size() != 16)
	{
		return;
	}

	std::array<double,16> tempVals;

	auto matIt = tempVals.begin();
	for(const auto& value : values)
	{
		bool ok = false;
		const auto val = value.toDouble(&ok);

		if(!ok)
		{
			return;
		}

		(*matIt++) = val;
	}

	mMatrixData = std::move(tempVals);

	updateMatrixValues();
}


void QGMDialogMatrix::getValues(std::vector<double>& values) const
{
	values.resize(16);
	auto it = mLineEditPtrs.begin();

	for(auto& value : values)
	{
		value = (*it++)->text().toDouble();
	}
}

void QGMDialogMatrix::updateMatrixValues()
{
	auto it = mLineEditPtrs.begin();

	for(auto value : mMatrixData)
	{
		(*it++)->setText(QString::number(value));
	}
}

void QGMDialogMatrix::tabChanged(int index)
{
	bool enable = index == 0;

	for(auto matrixField : mLineEditPtrs)
	{
		matrixField->setEnabled(enable);
	}
}

void QGMDialogMatrix::updateScale(double value)
{
	if(ui->checkBox_scaleUniform->isChecked())
	{
		ui->doubleSpinBox_scaleX->blockSignals(true);
		ui->doubleSpinBox_scaleY->blockSignals(true);
		ui->doubleSpinBox_scaleZ->blockSignals(true);

		ui->doubleSpinBox_scaleX->setValue(value);
		ui->doubleSpinBox_scaleY->setValue(value);
		ui->doubleSpinBox_scaleZ->setValue(value);

		ui->doubleSpinBox_scaleX->blockSignals(false);
		ui->doubleSpinBox_scaleY->blockSignals(false);
		ui->doubleSpinBox_scaleZ->blockSignals(false);
	}

	const double scaleX = ui->doubleSpinBox_scaleX->value();
	const double scaleY = ui->doubleSpinBox_scaleY->value();
	const double scaleZ = ui->doubleSpinBox_scaleZ->value();

	mMatrixData = {scaleX,    0.0,    0.0, 0.0,
	                  0.0, scaleY,    0.0, 0.0,
	                  0.0,    0.0, scaleZ, 0.0,
	                  0.0,    0.0,    0.0, 1.0};

	updateMatrixValues();
}

void QGMDialogMatrix::updateTranslate()
{
	const double xVal = ui->doubleSpinBox_transX->value();
	const double yVal = ui->doubleSpinBox_transY->value();
	const double zVal = ui->doubleSpinBox_transZ->value();

	mMatrixData = {1.0 ,0.0 ,0.0 ,0.0,
	               0.0 ,1.0 ,0.0 ,0.0,
	               0.0 ,0.0 ,1.0 ,0.0,
	               xVal,yVal,zVal,1.0};

	updateMatrixValues();
}

std::array<double,16> getAngleAxisMat(double aX, double aY, double aZ, double s, double c)
{
	const double len = std::sqrt(aX * aX + aY * aY + aZ * aZ);
	aX /= len;
	aY /= len;
	aZ /= len;

	const double mC = (1.0 - c);

	return {     c + aX * aX * mC,     aY*aX*mC + aZ * s,     aZ*aX*mC - aY * s, 0.0,
		    aX * aY * mC - aZ * s,      c + aY * aY * mC, aZ * aY * mC + aX * s, 0.0,
		    aX * aZ * mC + aY * s, aY * aZ * mC - aX * s,      c + aZ * aZ * mC, 0.0,
		    0.0, 0.0, 0.0, 1.0};
}

void QGMDialogMatrix::updateRotate(double angle)
{
	//update slider
	ui->horizontalSlider_angle->blockSignals(true);
	const double sliderMax = static_cast<double>(ui->horizontalSlider_angle->maximum());
	const double sliderMin = static_cast<double>(ui->horizontalSlider_angle->minimum());
	const double percent = (angle - ui->doubleSpinBox_angle->minimum()) / (ui->doubleSpinBox_angle->maximum() - ui->doubleSpinBox_angle->minimum());

	ui->horizontalSlider_angle->setValue(percent * (sliderMax - sliderMin) + sliderMin);
	ui->horizontalSlider_angle->blockSignals(false);

	const double s = sin(angle * M_PI / 180.0);
	const double c = cos(angle * M_PI / 180.0);

	switch (ui->buttonGroup_axisSel->checkedId()) {
		case 0: // x-axis
			mMatrixData = {1.0, 0.0, 0.0, 0.0,
			               0.0,   c,   s, 0.0,
			               0.0,  -s,   c, 0.0,
			               0.0, 0.0, 0.0, 1.0};
			break;
		case 1: // y-axis
			mMatrixData = {  c, 0.0,  -s, 0.0,
			               0.0, 1.0, 0.0, 0.0,
			                 s, 0.0,   c, 0.0,
			               0.0, 0.0, 0.0, 1.0};
			break;
		case 2: // z-axis
			mMatrixData = {  c,   s, 0.0, 0.0,
			                -s,   c, 0.0, 0.0,
			               0.0, 0.0, 1.0, 0.0,
			               0.0, 0.0, 0.0, 1.0};
			break;
		case 3: // free-axis
			//avoid case, where all axis values are 0.0
			if(std::abs(ui->doubleSpinBox_axisX->value()) <= std::numeric_limits<double>::epsilon() &&
			   std::abs(ui->doubleSpinBox_axisY->value()) <= std::numeric_limits<double>::epsilon() &&
			   std::abs(ui->doubleSpinBox_axisZ->value()) <= std::numeric_limits<double>::epsilon())
			{
				return;
			}
			mMatrixData = getAngleAxisMat(ui->doubleSpinBox_axisX->value(),
			                              ui->doubleSpinBox_axisY->value(),
			                              ui->doubleSpinBox_axisZ->value(),
			                              s,c);
			break;
	}

	updateMatrixValues();
}

void QGMDialogMatrix::updateRotateBySlider(int value)
{
	const double angleMin = ui->doubleSpinBox_angle->minimum();
	const double angleMax = ui->doubleSpinBox_angle->maximum();
	const double angleRange = angleMax - angleMin;

	const double valRel     = static_cast<double>(value - ui->horizontalSlider_angle->minimum());
	const double valueRange = static_cast<double>(ui->horizontalSlider_angle->maximum() - ui->horizontalSlider_angle->minimum());

	ui->doubleSpinBox_angle->setValue( (valRel / valueRange * angleRange) + angleMin );
}

void QGMDialogMatrix::resetValues()
{
	mMatrixData = {1.0,0.0,0.0,0.0,
	               0.0,1.0,0.0,0.0,
	               0.0,0.0,1.0,0.0,
	               0.0,0.0,0.0,1.0};

	updateMatrixValues();
}

void QGMDialogMatrix::copyToClipboard() const
{
	QString clipBoardText;

	for(auto value : mLineEditPtrs)
	{
		clipBoardText += QString("%1 ").arg(value->text());
	}

	QApplication::clipboard()->setText(clipBoardText);
}
