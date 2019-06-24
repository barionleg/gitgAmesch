#include "ExternalProgramsDialog.h"
#include "ui_ExternalProgramsDialog.h"
#include <QFileDialog>
#include <QStyle>

ExternalProgramsDialog::ExternalProgramsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ExternalProgramsDialog),
	mPdfLatexPath(""),
	mPdfViewerPath(""),
	mInkscapePath("")
{
	ui->setupUi(this);

	connect(ui->inkscape_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(inkscapePathChanged(const QString&)));
	connect(ui->pdf_latex_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pdfLatexPathChanged(const QString&)));
	connect(ui->pdf_viewer_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pdfViewerPathChanged(const QString&)));
	connect(ui->python_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pythonPathChanged(const QString&)));

	connect(ui->inkscape_pushButton, SIGNAL(pressed()), this, SLOT(inkscapePathChoose()));
	connect(ui->pdf_latex_pushButton, SIGNAL(pressed()), this, SLOT(pdfLatexPathChoose()));
	connect(ui->pdf_viewer_pushButton, SIGNAL(pressed()), this, SLOT(pdfViewerPathChoose()));
	connect(ui->python_pushButton, SIGNAL(pressed()), this, SLOT(pythonPathChoose()));

	connect(ui->defaultButton, SIGNAL(pressed()), this, SLOT(defaultPressed()));
}

ExternalProgramsDialog::~ExternalProgramsDialog()
{
	delete ui;
}

std::string ExternalProgramsDialog::pdfLatexPath() const
{
	return mPdfLatexPath;
}

void ExternalProgramsDialog::setPdfLatexPath(const std::string& pdfLatexPath)
{
	mPdfLatexPath = pdfLatexPath;
	ui->pdf_latex_lineEdit->setText(pdfLatexPath.c_str());
}

std::string ExternalProgramsDialog::pdfViewerPath() const
{
	return mPdfViewerPath;
}

void ExternalProgramsDialog::setPdfViewerPath(const std::string& pdfViewerPath)
{
	mPdfViewerPath = pdfViewerPath;
	ui->pdf_viewer_lineEdit->setText(pdfViewerPath.c_str());
}

std::string ExternalProgramsDialog::inkscapePath() const
{
	return mInkscapePath;
}

void ExternalProgramsDialog::setInkscapePath(const std::string& inkscapePath)
{
	mInkscapePath = inkscapePath;
	ui->inkscape_lineEdit->setText(inkscapePath.c_str());
}

std::string ExternalProgramsDialog::pythonPath() const
{
	return mPythonPath;
}

void ExternalProgramsDialog::setPythonPath(const std::string& pythonPath)
{
	mPythonPath = pythonPath;
	ui->python_lineEdit->setText(pythonPath.c_str());
}

void ExternalProgramsDialog::inkscapePathChanged(const QString& string)
{
	mInkscapePath = string.toStdString();
}

void ExternalProgramsDialog::pdfLatexPathChanged(const QString& string)
{
	mPdfLatexPath = string.toStdString();
}

void ExternalProgramsDialog::pdfViewerPathChanged(const QString& string)
{
	mPdfViewerPath = string.toStdString();
}

void ExternalProgramsDialog::pythonPathChanged(const QString& string)
{
	mPythonPath = string.toStdString();
}

void choosePath(const QString& header, QLineEdit* lineEditElement, QWidget* parent)
{
	auto fileName = QFileDialog::getOpenFileName(parent, header);

	if(fileName.length() > 0)
	{
		#ifdef WIN32
			fileName = "\"" + fileName + "\"";
		#endif
		lineEditElement->setText(fileName);
	}
}

void ExternalProgramsDialog::inkscapePathChoose()
{
	choosePath(tr("Select Inkscape executable"), ui->inkscape_lineEdit, this);
}

void ExternalProgramsDialog::pdfLatexPathChoose()
{
	choosePath(tr("Select pdflatex executable"), ui->pdf_latex_lineEdit, this);
}

void ExternalProgramsDialog::pdfViewerPathChoose()
{
	choosePath(tr("Select a program to view pdf files"), ui->pdf_viewer_lineEdit, this);
}

void ExternalProgramsDialog::pythonPathChoose()
{
	choosePath(tr("Select python3 executable"), ui->python_lineEdit, this);
}

void ExternalProgramsDialog::defaultPressed()
{
	ui->inkscape_lineEdit->setText("inkscape");
	ui->pdf_latex_lineEdit->setText("pdflatex");
	ui->pdf_viewer_lineEdit->setText("evince");
	ui->python_lineEdit->setText("python3");
}

