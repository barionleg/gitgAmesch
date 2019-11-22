#ifndef SVGPATH_H
#define SVGPATH_H

#include "SvgElement.h"
#include <vector>
#include <list>
#include <sstream>

class SvgPath : public SvgElement
{
	public:
		enum LineCap {
			CAP_UNSET,
			CAP_BUTT,
			CAP_ROUND,
			CAP_SQUARE,
		};

		enum LineJoin {
			JOIN_UNSET,
			JOIN_MITER,
			JOIN_ROUND,
			JOIN_BEVEL
		};

		SvgPath();
		~SvgPath() override = default;

		void moveTo(double x, double y);
		void moveToRel(double x, double y);
		void lineTo(double x, double y);
		void lineToRel(double x, double y);
		void closePath();

		void setLineWidth(float width);
		void setLineCap(LineCap cap);
		void setLineJoin(LineJoin join);
		void setLineDash(const double* dashes, int num_dashes, double offset);
		void setLineDash(const std::vector<double>& dashes, double offset);

		void setColor(double r, double g, double b, double a = 1.0);
		void setScale(double xScale, double yScale);

		void setFilled(bool fill);
	private:
		std::stringstream mElementsStream;

		float mLineWidth;
		LineCap mCapStyle;
		LineJoin mJoinStyle;

		std::vector<double> mDashes;
		double mDashOffset;
		double mR;
		double mG;
		double mB;
		double mA;

		double mXScale;
		double mYScale;

		bool mIsFilled;
		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGPATH_H
