#include "NormalSphereSelectionDialog.h"
#include "ui_NormalSphereSelectionDialog.h"

#include "meshQt.h"
#include <QSlider>
#include <list>
#include <QAbstractButton>
#include <iostream>

NormalSphereSelectionDialog::NormalSphereSelectionDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::NormalSphereSelectionDialog),
	mMesh(nullptr)
{
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NormalSphereSelectionDialog::comboButtonBoxClicked);
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
