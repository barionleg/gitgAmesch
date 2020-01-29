#ifndef SVGTEXT_H
#define SVGTEXT_H

#include <string>
#include "SvgElement.h"

class SvgText : public SvgElement
{
	public:

		enum TextAnchor {
			ANCHOR_START,
			ANCHOR_MIDDLE,
			ANCHOR_END
		};

		SvgText();

		void setText(const std::string& text);
		void setText(const std::string& text, int x, int y);

		void setTextAnchor(TextAnchor anchor);

		void setPosition(double x, double y);
		void setSize(double fontSize);
		void setFont(const std::string& font);
		void setRotation(double angle);
	private:
		double mX;
		double mY;
		double mFontSize;
		double mRotation;

		TextAnchor mAnchor;

		std::string mText;
		std::string mFont;

		// SvgElement interface
	public:
		virtual std::string getString() const override;
};

#endif // SVGTEXT_H
