#include "SvgRect.h"
#include <sstream>

SvgRect::SvgRect() : mX(0.0), mY(0.0), mWidth(0.0), mHeight(0.0), mStrokeWidth(1.0), mIsFilled(false)
{

}

SvgRect::SvgRect(double x, double y, double width, double height) : mX(x), mY(y), mWidth(width), mHeight(height), mStrokeWidth(1.0), mIsFilled(false)
{

}

void SvgRect::setSize(double width, double height)
{
	mWidth = width;
	mHeight = height;
}

void SvgRect::setPosition(double x, double y)
{
	mX = x;
	mY = y;
}

void SvgRect::setFill(bool fill)
{
	mIsFilled = fill;
}

void SvgRect::setStrokeWidth(double width)
{
	mStrokeWidth = width;
}

std::string SvgRect::getString() const
{
	std::stringstream sStream;

	std::string fillString = "none";

	if(mIsFilled)
		fillString = "rgb(0%,0%,0%)";

	sStream << "<rect x=\"" << mX << "\" y=\"" << mY << "\"  width=\"" << mWidth << "\" height=\"" << mHeight << "\" style=\"fill:" << fillString << ";stroke-width:" << mStrokeWidth << ";stroke-linecap:square;stroke-linejoin:miter;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;\" />";

	return sStream.str();
}
