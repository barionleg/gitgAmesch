#ifndef SVGRECT_H
#define SVGRECT_H

#include "SvgElement.h"

class SvgRect : public SvgElement
{
	public:
		SvgRect();
		SvgRect(double x, double y, double width, double height);

		void setSize(double width, double height);
		void setPosition(double x, double y);
		void setFill(bool fill);
		void setStrokeWidth(double width);
	private:
		double mX;
		double mY;
		double mWidth;
		double mHeight;
		double mStrokeWidth;

		bool mIsFilled;

		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGRECT_H
