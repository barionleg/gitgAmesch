#include "dialogGridCenterSelect.h"
#include "ui_dialogGridCenterSelect.h"

#include <iostream>

dialogGridCenterSelect::dialogGridCenterSelect(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::dialogGridCenterSelect),
	mCenterPos(CenterPos::CENTER)
{
	ui->setupUi(this);

	connect(ui->centerSelectBtnGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(selectPos(QAbstractButton*, bool)));
}

dialogGridCenterSelect::~dialogGridCenterSelect()
{
	delete ui;
}

dialogGridCenterSelect::CenterPos dialogGridCenterSelect::centerPos() const
{
	return mCenterPos;
}

void dialogGridCenterSelect::setCenterPos(const CenterPos& centerPos)
{
	mCenterPos = centerPos;

	switch (centerPos) {
		case TOP_LEFT:
			ui->radioButton_TL->setChecked(true);
			break;
		case TOP:
			ui->radioButton_T->setChecked(true);
			break;
		case TOP_RIGHT:
			ui->radioButton_TR->setChecked(true);
			break;
		case LEFT:
			ui->radioButton_L->setChecked(true);
			break;
		case CENTER:
			ui->radioButton_C->setChecked(true);
			break;
		case RIGHT:
			ui->radioButton_R->setChecked(true);
			break;

		case BOTTOM_LEFT:
			ui->radioButton_BL->setChecked(true);
			break;
		case BOTTOM:
			ui->radioButton_B->setChecked(true);
			break;
		case BOTTOM_RIGHT:
			ui->radioButton_BR->setChecked(true);
			break;
	}
}

void dialogGridCenterSelect::selectPos(QAbstractButton* button, bool active)
{
	if(!active)
		return;

	if(button == ui->radioButton_TL)
		mCenterPos = CenterPos::TOP_LEFT;
	else if(button == ui->radioButton_T)
		mCenterPos = CenterPos::TOP;
	else if(button == ui->radioButton_TR)
		mCenterPos = CenterPos::TOP_RIGHT;

	else if(button == ui->radioButton_R)
		mCenterPos = CenterPos::RIGHT;
	else if(button == ui->radioButton_C)
		mCenterPos = CenterPos::CENTER;
	else if(button == ui->radioButton_L)
		mCenterPos = CenterPos::LEFT;

	else if(button == ui->radioButton_BL)
		mCenterPos = CenterPos::BOTTOM_LEFT;
	else if(button == ui->radioButton_B)
		mCenterPos = CenterPos::BOTTOM;
	else if(button == ui->radioButton_BR)
		mCenterPos = CenterPos::BOTTOM_RIGHT;
}
