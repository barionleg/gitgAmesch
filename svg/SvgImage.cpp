#include "SvgImage.h"
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <sstream>

unsigned int SvgImage::imageIDs = 0;

SvgImage::SvgImage() : mWidth(0), mHeight(0), mXOffset(0.0), mYOffset(0.0), mID(imageIDs++), mXScale(1.0), mYScale(1.0)
{

}

void SvgImage::setImage(const unsigned char* data, unsigned int width, unsigned int height, unsigned int numComponents, double x, double y)
{
	QImage img(data, width, height, numComponents * width * sizeof (unsigned char), QImage::Format_RGBA8888);

	QByteArray ba;
	QBuffer bu(&ba);
	bu.open(QIODevice::WriteOnly);
	img.save(&bu, "PNG");
	mImgBase64String = ba.toBase64().toStdString();

	mWidth = width;
	mHeight = height;
	mXOffset = x;
	mYOffset = y;
}

void SvgImage::setScale(double xScale, double yScale)
{
	mXScale = xScale; mYScale = yScale;
}


std::string SvgImage::getString() const
{
	std::stringstream sStream;

	sStream << "<image id=\"image" << mID
	        << "\" width=\"" << mWidth
	        << "\" height=\"" << mHeight
			<< "\" xlink:href=\"data:image/png;base64," << mImgBase64String << "\" ";

	if(mXScale != 1.0 || mYScale != 1.0)
	{
		sStream << " transform=\"scale(" << mXScale << "," << mYScale <<")\"";
	}

	sStream << "/>" ;

	return sStream.str();
}
