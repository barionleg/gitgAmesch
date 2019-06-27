#include "NormalSphereSelectionDialog.h"
#include "ui_NormalSphereSelectionDialog.h"

#include "meshQt.h"
#include <QSlider>
#include <list>
#include <QAbstractButton>
#include <QRadioButton>
#include <iostream>

NormalSphereSelectionDialog::NormalSphereSelectionDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::NormalSphereSelectionDialog),
	mMesh(nullptr)
{
	ui->setupUi(this);

	ui->colorMap_comboBox->addItem(tr("Grayscale"));
	ui->colorMap_comboBox->addItem(tr("Hot"));
	ui->colorMap_comboBox->addItem(tr("Cold"));
	ui->colorMap_comboBox->addItem(tr("HSV (full)"));
	ui->colorMap_comboBox->addItem(tr("HSV (part)"));
	ui->colorMap_comboBox->addItem(tr("Brewer RdGy"));
	ui->colorMap_comboBox->addItem(tr("Brewer Spectral"));
	ui->colorMap_comboBox->addItem(tr("Brewer RdYlGn"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric Tint"));
	ui->colorMap_comboBox->addItem(tr("Jet (Octave)"));
	ui->colorMap_comboBox->addItem(tr("Morgenstemning"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric HiRise1"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric HiRise2"));
	ui->colorMap_comboBox->addItem(tr("Parula"));
	ui->colorMap_comboBox->addItem(tr("Brewer YlOrBr"));
	ui->colorMap_comboBox->addItem(tr("Copper (Octave)"));
	ui->colorMap_comboBox->addItem(tr("Rust"));
	ui->colorMap_comboBox->addItem(tr("Sienna"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric Arid"));

	connect(ui->colorMap_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {this->ui->openGLWidget->setColorMapIndex(index);});
	connect(ui->selectionRadius_horizontalSlider, &QSlider::valueChanged, [this](int value) {this->ui->openGLWidget->setSelectionRadius(value);});

	connect(ui->selectionModeSelect_radioButton, &QRadioButton::pressed, [this](){this->ui->openGLWidget->setSelectionMask(255);});
	connect(ui->selectionModeClear_radioButton, &QRadioButton::pressed, [this](){this->ui->openGLWidget->setSelectionMask(0);});

	ui->colorMap_comboBox->setCurrentIndex(11);

	connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NormalSphereSelectionDialog::comboButtonBoxClicked);

	connect(ui->openGLWidget, &NormalSphereSelectionRenderWidget::rotationChanged, this, &NormalSphereSelectionDialog::renderWidgetRotationChanged);

	connect(ui->rotationSharing_checkBox, &QRadioButton::toggled,
			[this](bool enabled){
									if(enabled) {
										emit rotationChanged(this->ui->openGLWidget->getRotation());}
								});

	connect(ui->normalScaling_checkBox, &QCheckBox::stateChanged, [this](int state){this->ui->openGLWidget->setScaleNormals(state != Qt::Unchecked);});
}

NormalSphereSelectionDialog::~NormalSphereSelectionDialog()
{
	delete ui;
}

void NormalSphereSelectionDialog::setMeshNormals(MeshQt* mesh)
{
	mMesh = mesh;
	std::vector<Vertex*> vertices;
	mesh->getVertexList(&vertices);

	std::vector<float> normalData;
	normalData.reserve(vertices.size() * 3);

	for(const auto vertex : vertices)
	{
		normalData.push_back(vertex->getNormalX());
		normalData.push_back(vertex->getNormalY());
		normalData.push_back(vertex->getNormalZ());
	}

	ui->openGLWidget->setRenderNormals(normalData);

}

void NormalSphereSelectionDialog::selectMeshByNormals()
{
	if(mMesh == nullptr)
		return;

	std::vector<Vertex*> vertices;
	mMesh->getVertexList(&vertices);

	std::list<Vertex*> selectedVertices;

	for(const auto vertex : vertices)
	{

		if(ui->openGLWidget->isNormalSelected(vertex->getNormalX(), vertex->getNormalY(), vertex->getNormalZ()))
		{
			selectedVertices.push_back(vertex);
		}
	}

	if(selectedVertices.empty())
		return;

	std::vector<Vertex*> selVertexVec;
	selVertexVec.reserve(selectedVertices.size());
	std::copy(selectedVertices.begin(), selectedVertices.end(), std::back_inserter(selVertexVec));


	mMesh->addToSelection(&selVertexVec);
}

void NormalSphereSelectionDialog::comboButtonBoxClicked(QAbstractButton* button)
{
	if(button == ui->buttonBox->button(QDialogButtonBox::Apply))
		selectMeshByNormals();

	else if(button == ui->buttonBox->button(QDialogButtonBox::Reset))
		ui->openGLWidget->clearSelected();
}

void NormalSphereSelectionDialog::renderWidgetRotationChanged(QQuaternion quat)
{
	if(ui->rotationSharing_checkBox->isChecked())
	{
		emit rotationChanged(quat);
	}
}

void NormalSphereSelectionDialog::updateRotationExternal(Vector3D camCenter, Vector3D camUp)
{
	if(ui->rotationSharing_checkBox->isChecked())
	{
		QVector3D target = QVector3D(camCenter.getX(), camCenter.getY(), camCenter.getZ()).normalized();


		QQuaternion rotQuat = QQuaternion::fromDirection(target, QVector3D(camUp.getX(), camUp.getY(), camUp.getZ())).conjugated();
		ui->openGLWidget->setRotation(rotQuat);
	}
}


void NormalSphereSelectionDialog::changeEvent(QEvent* event)
{
	switch(event->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;

		default:
			break;
	}

	QDialog::changeEvent(event);
}
