#ifndef SVGCIRCLE_H
#define SVGCIRCLE_H

#include <array>

#include "SvgElement.h"

class SvgCircle : public SvgElement
{
	public:
		SvgCircle();
		SvgCircle(double posX, double posY, double radius);

		void setFilled(bool filled);
		void setStroke(bool hasStroke);

		void setPosition(double posX, double posY);
		void setRadius(double radius);
		void setFillColor(double r, double g, double b);
		void setStrokeColor(double r, double g, double b);
	private:
		double mX;
		double mY;
		double mRadius;

		std::array<double, 3> mFillRGB;
		std::array<double, 3> mStrokeRGB;

		bool mIsFilled;
		bool mHasStroke;
		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGCIRCLE_H
