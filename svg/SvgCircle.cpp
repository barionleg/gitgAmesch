#include "SvgCircle.h"
#include <sstream>

SvgCircle::SvgCircle() : mX(0.0), mY(0.0), mRadius(0.0), mFillRGB{0,0,0}, mStrokeRGB{0,0,0}, mIsFilled(true), mHasStroke(false)
{

}

SvgCircle::SvgCircle(double posX, double posY, double radius) : mX(posX), mY(posY), mRadius(radius), mFillRGB{0,0,0}, mStrokeRGB{0,0,0}, mIsFilled(true), mHasStroke(false)
{

}

void SvgCircle::setFilled(bool filled)
{
	mIsFilled = filled;
}

void SvgCircle::setStroke(bool hasStroke)
{
	mHasStroke = hasStroke;
}

void SvgCircle::setPosition(double posX, double posY)
{
	mX = posX; mY = posY;
}

void SvgCircle::setRadius(double radius)
{
	mRadius = radius;
}

void SvgCircle::setFillColor(double r, double g, double b)
{
	mFillRGB[0] = r;
	mFillRGB[1] = g;
	mFillRGB[2] = b;
}

void SvgCircle::setStrokeColor(double r, double g, double b)
{
	mStrokeRGB[0] = r;
	mStrokeRGB[1] = g;
	mStrokeRGB[2] = b;
}


std::string SvgCircle::getString() const
{
	std::stringstream sStream;

	std::string fillString("none");
	std::string strokeString("none");

	if(mIsFilled)
	{
		std::stringstream rgbText;
		rgbText << "rgb(" << mFillRGB[0] * 100.0 << "%," << mFillRGB[1] * 100.0 << "%," << mFillRGB[2] * 100.0 << "%)";
		fillString = rgbText.str();
	}

	if(mHasStroke)
	{
		std::stringstream rgbText;
		rgbText << "rgb(" << mStrokeRGB[0] * 100.0 << "%," << mStrokeRGB[1] * 100.0 << "%," << mStrokeRGB[2] * 100.0 << "%)";
		strokeString = rgbText.str();
	}

	sStream << "<circle cx=\"" << mX << "\" cy=\"" << mY << "\" r=\"" << mRadius << "\" fill=\"" << fillString << "\" stroke=\"" << strokeString << "\" />";

	return sStream.str();
}
