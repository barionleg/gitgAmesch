#include "SvgWriter.h"

#include <fstream>
#include <memory>

SvgWriter::SvgWriter() : mWidth(0), mHeight(0)
{

}

SvgWriter::~SvgWriter()
{

}

void SvgWriter::setSize(double width, double height)
{
	mWidth = width;
	mHeight = height;
}

void SvgWriter::addElement(std::unique_ptr<SvgElement>&& element)
{
	mElements.push_back(std::move(element));
}

void SvgWriter::writeToFile(const std::string& filename)
{
	//write to filename
	std::ofstream fStream(filename);

	//write Header
	fStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" ;
	fStream << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";

	if(mWidth != 0 || mHeight != 0)
	{
		fStream << " width=\"" << mWidth << "pt\" height=\"" << mHeight << "pt\" viewBox=\"0 0 " << mWidth << " " << mHeight << "\"";
	}

	fStream << " version=\"1.1\">\n";
	//write components
	for(const auto& element : mElements)
	{
		fStream << element->getString() << "\n";
	}

	//closing
	fStream << "</svg>\n";

	fStream.close();
}

void SvgWriter::startGroup()
{
	auto startGroup = std::make_unique<DummySvgElement>("<g>");
	mElements.push_back(std::move(startGroup));
}

void SvgWriter::endGroup()
{
	auto endGroup = std::make_unique<DummySvgElement>("</g>");
	mElements.push_back(std::move(endGroup));
}

void SvgWriter::startDefSection()
{

}

void SvgWriter::endDefSection()
{

}


SvgWriter::DummySvgElement::DummySvgElement(const std::string& content) : mContent(content)
{

}

std::string SvgWriter::DummySvgElement::getString() const
{
	return mContent;
}
