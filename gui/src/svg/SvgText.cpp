#include "SvgText.h"
#include <sstream>

SvgText::SvgText() : mX(0), mY(0), mFontSize(-1.0), mRotation(0.0), mAnchor(TextAnchor::ANCHOR_START)
{

}

void SvgText::setText(const std::string& text)
{
	mText = text;
}

void SvgText::setText(const std::string& text, int x, int y)
{
	mText = text;
	mX = x;
	mY = y;
}

void SvgText::setTextAnchor(SvgText::TextAnchor anchor)
{
	mAnchor = anchor;
}

void SvgText::setPosition(double x, double y)
{
	mX = x;
	mY = y;
}

void SvgText::setSize(double fontSize)
{
	mFontSize = fontSize;
}

void SvgText::setFont(const std::string& font)
{
	mFont = font;
}

void SvgText::setRotation(double angle)
{
	mRotation = angle;
}

std::string SvgText::getString() const
{
	std::stringstream sStream;

	sStream << "<text x=\"" << mX
	        << "\" y=\"" << mY
			<< "\" ";

	if(!mFont.empty() || mFontSize >= 0.0)
	{
		sStream << "style=\"";
		if(!mFont.empty())
		{
			sStream << "font-family:" << mFont << ";";
		}
		if(mFontSize >= 0.0)
		{
			sStream << "font-size:" << mFontSize << "px;";
		}
		sStream << "\"";
	}

	if(mAnchor != TextAnchor::ANCHOR_START)
	{
		switch (mAnchor) {
			case TextAnchor::ANCHOR_MIDDLE:
				sStream << R"( text-anchor="middle")";
				break;
			case TextAnchor::ANCHOR_END:
				sStream << R"( text-anchor="end")";
				break;
			default:
				break;
		}

	}

	if(mRotation != 0.0)
	{
		sStream << R"( transform="rotate( )" << mRotation << " " << mX << " " << mY << ")\"";
	}

	sStream << ">" << mText << "</text>";
	return sStream.str();
}
