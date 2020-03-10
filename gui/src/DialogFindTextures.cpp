#include "DialogFindTextures.h"
#include "ui_DialogFindTextures.h"

#include <QGLWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QImageReader>

DialogFindTextures::DialogFindTextures(const QStringList& missingTextures, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFindTextures)
{
	ui->setupUi(this);

	for(auto textureIt = missingTextures.begin(); textureIt != missingTextures.end(); ++textureIt)
	{
		auto selectWidget = new FileSelectWidget(*textureIt);
		ui->fileSelectLayout->addWidget(selectWidget);
		mSelectWidgets.push_back(selectWidget);
	}
}


DialogFindTextures::~DialogFindTextures()
{
	delete ui;
}

QStringList DialogFindTextures::getFileNames()
{
	QStringList fileNames;

	for(auto it = mSelectWidgets.begin(); it != mSelectWidgets.end(); ++it)
	{
		fileNames.push_back((*it)->getFileName());
	}

	return fileNames;
}

QString FileSelectWidget::sSupportedImageFormats = QString();

FileSelectWidget::FileSelectWidget(const QString& fileName, QWidget* parent) : QWidget(parent), mFile(fileName)
{
	auto layout = new QHBoxLayout;
	layout->setMargin(0);

	this->setLayout(layout);

	QFileInfo fileInfo(fileName);

	mFileNameLabel = new QLabel(fileInfo.fileName());
	layout->addWidget(mFileNameLabel);

	auto dialogButton = new QPushButton(QString("..."));
	layout->addWidget(dialogButton);

	connect(dialogButton, &QPushButton::clicked, this, &FileSelectWidget::selectFile);
}

QString FileSelectWidget::getFileName() const {
	return mFile;
}

void FileSelectWidget::selectFile()
{
	if(sSupportedImageFormats.isNull())
	{
		auto imgFiles = QImageReader::supportedImageFormats();

		for(const auto& imgType : imgFiles)
		{
			sSupportedImageFormats += QString("*." + imgType + " ");
		}
	}

	auto fileName = QFileDialog::getOpenFileName(this, tr("select Texture for") + " " + mFileNameLabel->text(),
	                                             "",tr("Images") + "(" + sSupportedImageFormats + ")");

	if(!fileName.isNull())
	{
		QFileInfo fileInfo(fileName);
		mFileNameLabel->setText(fileInfo.fileName());
		mFile = fileName;
	}
}
