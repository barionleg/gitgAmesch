#include "QGMDialogMatrix.h"
#include "ui_QGMDialogMatrix.h"

#include <QClipboard>
#include <QDoubleValidator>

QGMDialogMatrix::QGMDialogMatrix(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QGMDialogMatrix),
    mValidator(new QDoubleValidator)
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

	for(auto lineEditPtr : mLineEditPtrs)
	{
		lineEditPtr->setValidator(mValidator.get());
	}
}

QGMDialogMatrix::~QGMDialogMatrix()
{
	delete ui;
}

void QGMDialogMatrix::fetchClipboard()
{
	QClipboard* clipboard = QApplication::clipboard();
	QString clipBoardStr = clipboard->text( QClipboard::Clipboard );

	QStringList values = clipBoardStr.simplified().split( " ", QString::SkipEmptyParts);

	if(values.size() != 16)
	{
		return;
	}

	auto matIt = mMatrixData.begin();
	for(const auto& value : values)
	{
		bool ok = false;
		auto val = value.toDouble(&ok);

		if(ok)
		{
			(*matIt) = val;
		}
		++matIt;
	}

	updateMatrixValues();
}

void QGMDialogMatrix::getValues(std::vector<double>& values) const
{
	values.resize(16);
	auto it = mLineEditPtrs.begin();

	for(auto& value : values)
	{
		value = (*it)->text().toDouble();
		++it;
	}
}

void QGMDialogMatrix::updateMatrixValues()
{
	auto it = mLineEditPtrs.begin();

	for(auto value : mMatrixData)
	{
		(*it)->setText(QString::number(value));
		++it;
	}
}
