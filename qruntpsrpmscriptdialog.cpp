#include "qruntpsrpmscriptdialog.h"
#include "ui_qruntpsrpmscriptdialog.h"

#include <iostream>

#include <QFile>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCryptographicHash>
#include <QProcess>
#include <QSettings>

#include "QGMMacros.h"

#include "MeshQtCSVImportExport.h"

QRunTpsRpmScriptDialog::QRunTpsRpmScriptDialog(QWidget* parent,
                                                MeshGL* meshGLPtr) :
                                                    QDialog(parent),
                                                    m_qRunTpsRpmScriptDialog(new Ui::QRunTpsRpmScriptDialog)
{

	m_meshGLPtr = meshGLPtr;

	QSettings settings;

	m_lastOpenedFileDirectory = settings.value("lastPath").toString();

	m_qRunTpsRpmScriptDialog->setupUi(this);

	setWindowIcon(QIcon(_GIGAMESH_LOGO_));

	connect(m_qRunTpsRpmScriptDialog->inputCoordinateFileSelectionPushButton,
	                                                    &QPushButton::released,
	                                                    this,
	                                                    &QRunTpsRpmScriptDialog::
	                                                    meshCoordinatesOriginFileSelectionPushButtonReleased);

	connect(m_qRunTpsRpmScriptDialog->inputPositionsOriginFileSelectionPushButton,
	                                                        &QPushButton::released,
	                                                        this,
	                                                        &QRunTpsRpmScriptDialog::
	                                                        primitiveCoordinatesOriginFileSelectionPushButtonReleased);

	connect(m_qRunTpsRpmScriptDialog->inputPositionsTargetFileSelectionPushButton,
	                                                        &QPushButton::released,
	                                                        this,
	                                                        &QRunTpsRpmScriptDialog::
	                                                        primitiveCoordinatesTargetFileSelectionPushButtonReleased);

	connect(m_qRunTpsRpmScriptDialog->outputVertexFileSelectionPushButton,
	                                                &QPushButton::released,
	                                                this,
	                                                &QRunTpsRpmScriptDialog::
	                                                meshDataOutputFileSelectionPushButtonReleased);

	connect(m_qRunTpsRpmScriptDialog->ImportCoordinatesAfterTransformationCheckBox,
	                                                        &QCheckBox::stateChanged,
	                                                        this,
	                                                        &QRunTpsRpmScriptDialog::
	                                                        importGeneratedVerticesToCurrentMeshCheckBoxChanged);

	connect(m_qRunTpsRpmScriptDialog->applyOrCancelButtonBox->button(QDialogButtonBox::Apply),
	                                                            &QPushButton::released,
	                                                            this,
	                                                            &QRunTpsRpmScriptDialog::applyTpsRpmTransformation);


	m_qRunTpsRpmScriptDialog->applyOrCancelButtonBox->
	            button(QDialogButtonBox::Apply)->setEnabled(false);
}

QRunTpsRpmScriptDialog::~QRunTpsRpmScriptDialog()
{
	delete m_qRunTpsRpmScriptDialog;
}

void QRunTpsRpmScriptDialog::meshCoordinatesOriginFileSelectionPushButtonReleased()
{
	const QString coordinatesOriginFileName = QFileDialog::getOpenFileName(this,
	                                                    "Origin primitive positions "
	                                                    "CSV file used for "
	                                                    "TPS-RPM transformation",
	                                                    m_lastOpenedFileDirectory,
	                                                    "CSV files (*.csv *.mat)" );

	if(!coordinatesOriginFileName.isEmpty())
	{
		if(QFile::exists(coordinatesOriginFileName))
		{
			m_meshCoordinatesOriginFileName = coordinatesOriginFileName;

			m_qRunTpsRpmScriptDialog->inputCoordinateFileLineEdit->setText(
			                                                m_meshCoordinatesOriginFileName);

			m_meshCoordinatesOriginFileNameValid = true;
		}

		else
		{
			m_meshCoordinatesOriginFileNameValid = false;
		}
	}

	else
	{
		m_meshCoordinatesOriginFileNameValid = false;
	}

	checkInput();
}

void QRunTpsRpmScriptDialog::primitiveCoordinatesOriginFileSelectionPushButtonReleased()
{

	const QString positionsOriginFileName = QFileDialog::getOpenFileName(this,
	                                                    "Origin primitive positions "
	                                                    "CSV file used for "
	                                                    "TPS-RPM transformation",
	                                                    m_lastOpenedFileDirectory,
	                                                    "CSV files (*.csv *.mat)");

	if(!positionsOriginFileName.isEmpty())
	{
		if(QFile::exists(positionsOriginFileName))
		{
			m_primitiveCoordinatesOriginFileName = positionsOriginFileName;

			m_qRunTpsRpmScriptDialog->inputPositionsOriginFileLineEdit->setText(
			                                                m_primitiveCoordinatesOriginFileName);

			m_primitiveCoordinatesOriginFileNameValid = true;
		}

		else
		{
			m_primitiveCoordinatesOriginFileNameValid = false;
		}
	}

	else
	{
		m_primitiveCoordinatesOriginFileNameValid = false;
	}

	checkInput();
}

void QRunTpsRpmScriptDialog::primitiveCoordinatesTargetFileSelectionPushButtonReleased()
{

	const QString positionsTargetFileName = QFileDialog::getOpenFileName(this,
	                                                    "Target primitive positions "
	                                                    "CSV file used for "
	                                                    "TPS-RPM transformation",
	                                                    m_lastOpenedFileDirectory,
	                                                    "CSV files (*.csv *.mat)");

	if(!positionsTargetFileName.isEmpty())
	{
		if(QFile::exists(positionsTargetFileName))
		{
			m_primitiveCoordinatesTargetFileName = positionsTargetFileName;

			m_qRunTpsRpmScriptDialog->inputPositionsTargetFileLineEdit->setText(
			                                                m_primitiveCoordinatesTargetFileName);

			m_primitiveCoordinatesTargetFileNameValid = true;
		}

		else
		{
			m_primitiveCoordinatesTargetFileNameValid = false;
		}
	}

	else
	{
		m_primitiveCoordinatesTargetFileNameValid = false;
	}

	checkInput();
}

void QRunTpsRpmScriptDialog::meshDataOutputFileSelectionPushButtonReleased()
{
	const QString outputFileName = QFileDialog::getSaveFileName(this,
	                                                    "Output vertex coordinate "
	                                                    "CSV file of "
	                                                    "TPS-RPM transformation",
	                                                    m_lastOpenedFileDirectory,
	                                                    "CSV files (*.csv *.mat)");

	if(!outputFileName.isEmpty())
	{

		m_meshDataOutputFileName = outputFileName;

		m_qRunTpsRpmScriptDialog->outputVertexFileLineEdit->setText(
		                                                m_meshDataOutputFileName);

		m_meshDataOutputFileNameValid = true;

	}

	else
	{
		m_meshDataOutputFileNameValid = false;
	}

	checkInput();
}

void QRunTpsRpmScriptDialog::importGeneratedVerticesToCurrentMeshCheckBoxChanged(
                                                                    int newCheckState)
{
	m_importGeneratedMeshDataToCurrentMesh =
	        (newCheckState == Qt::CheckState::Checked) ? true : false;
}

void QRunTpsRpmScriptDialog::applyTpsRpmTransformation()
{

	QTemporaryFile tpsRpmScriptTempFile;

	tpsRpmScriptTempFile.setAutoRemove(true);

	if(tpsRpmScriptTempFile.open())
	{
		std::cout << "[Mesh::" << __FUNCTION__ << "] "
		        << "Opened TPS RPM script temp file "
		        << tpsRpmScriptTempFile.fileName().toStdString()
		        << std::endl;
	}

	else
	{
		std::clog << "[Mesh::" << __FUNCTION__ << "] "
		            << "Error: Could not open temp file"
		            << std::endl;
		return;
	}

	const QString tpsRpmScriptTempFileName = tpsRpmScriptTempFile.fileName();

	QFile tpsRpmScriptFile(":/pythonscripts/warp_vertices.py");

	if(tpsRpmScriptFile.open(QFile::ReadOnly))
	{
		std::cout << "[Mesh::" << __FUNCTION__ << "] "
		        << "Opened TPS RPM script file "
		        << tpsRpmScriptFile.fileName().toStdString()
		        << std::endl;
	}

	else
	{
		std::clog << "[Mesh::" << __FUNCTION__ << "] "
		            << "Error: Could not open temp file"
		            << std::endl;
		return;
	}

	tpsRpmScriptTempFile.write(tpsRpmScriptFile.readAll());

	tpsRpmScriptTempFile.flush();
	tpsRpmScriptTempFile.close();

	tpsRpmScriptFile.close();

	const QStringList tpsRpmScriptArguments({tpsRpmScriptTempFileName,
	                                            QString("--in-vertices"),
	                                            m_meshCoordinatesOriginFileName,
	                                            QString("--in-control-left"),
	                                            m_primitiveCoordinatesOriginFileName,
	                                            QString("--in-control-right"),
	                                            m_primitiveCoordinatesTargetFileName,
	                                            QString("--out-vertices"),
	                                            m_meshDataOutputFileName});

	QProcess* runTpsRpmScriptProcess = new QProcess(this);

	runTpsRpmScriptProcess->setProcessChannelMode( QProcess::ProcessChannelMode::ForwardedChannels );

	QSettings settings;
	QString pythonPath = settings.value("Python3_Path", "").toString();

	if(pythonPath.length() == 0)
		pythonPath = tr("python3");

	runTpsRpmScriptProcess->start(pythonPath,
	                                tpsRpmScriptArguments);

	runTpsRpmScriptProcess->waitForFinished(-1);

	if(m_importGeneratedMeshDataToCurrentMesh)
	{
		if(m_meshGLPtr != nullptr)
		{

			dynamic_cast<MeshQtCSVImportExport*>(m_meshGLPtr)->importVertexDataFromCSV(
			                                                            m_meshDataOutputFileName,
			                                                            [this]()
			                                                            {
				                                                            m_meshGLPtr->estBoundingBox();
			                                                            });
			m_meshGLPtr->resetVertexNormals();
			m_meshGLPtr->glRemove();
			m_meshGLPtr->glPrepare();
			m_meshGLPtr->changedVertFuncVal();
			m_meshGLPtr->setParamIntMeshGL(MeshGLParams::TEXMAP_CHOICE_FACES,
			                                    MeshGLParams::TEXMAP_VERT_FUNCVAL);
		}
	}

	delete runTpsRpmScriptProcess;

	close();
}

void QRunTpsRpmScriptDialog::checkInput()
{

	if(m_meshCoordinatesOriginFileNameValid &&
	    m_primitiveCoordinatesOriginFileNameValid &&
	    m_primitiveCoordinatesTargetFileNameValid &&
	    m_meshDataOutputFileNameValid)
	{
		m_qRunTpsRpmScriptDialog->applyOrCancelButtonBox->
		            button(QDialogButtonBox::Apply)->setEnabled(true);
	}

	else
	{
		m_qRunTpsRpmScriptDialog->applyOrCancelButtonBox->
		            button(QDialogButtonBox::Apply)->setEnabled(false);
	}

}

void showRunTpsRpmScriptDialog(QWidget* parent = nullptr,
                                    MeshGL* meshGLPtr = nullptr)
{
	QRunTpsRpmScriptDialog qRunTpsRpmScriptDialog(parent, meshGLPtr);

	qRunTpsRpmScriptDialog.exec();
}

