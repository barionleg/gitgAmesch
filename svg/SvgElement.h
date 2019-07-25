#ifndef SVGELEMENT_H
#define SVGELEMENT_H

#include <string>

//!
//! \brief SvgElement: Abstract class for all different types in the SVG-file (e.g. path, text, image...)
//!
class SvgElement
{
	public:
		SvgElement() = default;
		virtual ~SvgElement() = default;

		//!
		//! \brief getString Gets the current object as XML string
		//! \return The SVG-XML string
		//!
		virtual std::string getString() const = 0;
};

#endif // SVGELEMENT_H
