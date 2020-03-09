#ifndef SVGIMAGE_H
#define SVGIMAGE_H
#include <string>
#include "SvgElement.h"

class SvgImage : public SvgElement
{
	public:
		SvgImage();

		void setImage(const unsigned char* data, unsigned int width, unsigned int height, unsigned int numComponents, double x, double y);

		void setScale(double xScale, double yScale);
	private:
		static unsigned int imageIDs;

		std::string mImgBase64String;
		unsigned int mWidth;
		unsigned int mHeight;
		double mXOffset;
		double mYOffset;
		unsigned int mID;

		double mXScale;
		double mYScale;
		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGIMAGE_H
